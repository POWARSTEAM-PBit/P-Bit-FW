// src/ble.ts
// Web Bluetooth helper for P-BIT
// - New mode:  custom service/char (17B binary)
// - Legacy:    0x181A / 0x2A6E JSON

// ====== UUIDs (must match firmware) ======
const NEW_SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const NEW_CHAR_UUID    = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';

const LEGACY_SERVICE_UUID = 0x181a; // Environmental Sensing
const LEGACY_CHAR_UUID    = 0x2a6e; // Temperature (we send JSON)

export type Reading = {
  // unified fields used by Dashboard
  temp?: number;   // °C
  hum?: number;    // %
  ldr?: number;    // raw
  mic?: number;    // raw
  batt?: number;   // %
  // legacy/raw aliases so我们还能标注来源
  air_temp?: number;
  soil_temp?: number;
  air_hum?: number;
  soil_hum?: number;
  light?: number;
  noise?: number;
  ts: number;      // ms
};

// Extend Navigator to include the bluetooth property
interface Navigator {
  bluetooth: Bluetooth;
}

type Listener = (r: Reading) => void;

let device: BluetoothDevice | null = null;
let server: BluetoothRemoteGATTServer | null = null;
let newChar: BluetoothRemoteGATTCharacteristic | null = null;
let legacyChar: BluetoothRemoteGATTCharacteristic | null = null;

const subs = new Set<Listener>();

export function subscribe(fn: Listener) {
  subs.add(fn);
  return () => subs.delete(fn);
}

function emit(r: Reading) {
  for (const fn of subs) fn(r);
}

// ---- decode helpers ----

// New 17B binary: [0x02,0x00,(id,u16LE)*5]
// id: 1 temp(0.1C) 2 hum(0.1%) 3 ldr(raw) 4 mic(raw) 5 batt(%)
function parse17(buf: DataView): Reading | null {
  if (buf.byteLength !== 17) return null;
  if (buf.getUint8(0) !== 0x02) return null;

  const getVal = (idx: number) => {
    const base = 2 + idx * 3;
    const id   = buf.getUint8(base);
    const val  = buf.getUint16(base + 1, true);
    return { id, val };
  };

  const out: Reading = { ts: Date.now() };
  for (let i = 0; i < 5; i++) {
    const { id, val } = getVal(i);
    switch (id) {
      case 1: out.temp = val / 10; break;
      case 2: out.hum  = val / 10; break;
      case 3: out.ldr  = val;      break;
      case 4: out.mic  = val;      break;
      case 5: out.batt = val;      break;
      default: break;
    }
  }
  return out;
}

// Legacy JSON (UTF-8) sent via 0x2A6E
function parseLegacyJSON(buf: DataView): Reading | null {
  try {
    const txt = new TextDecoder().decode(buf.buffer);
    const j = JSON.parse(txt);

    const r: Reading = { ts: Date.now() };
    // keep both the “raw names” and the unified ones for dashboard mapping
    if (typeof j.temp === 'number') r.temp = j.temp;
    if (typeof j.hum  === 'number') r.hum  = j.hum;
    if (typeof j.ldr  === 'number') r.ldr  = j.ldr;
    if (typeof j.mic  === 'number') r.mic  = j.mic;
    if (typeof j.batt === 'number') r.batt = j.batt;

    if (typeof j.air_temp  === 'number') r.air_temp  = j.air_temp;
    if (typeof j.soil_temp === 'number') r.soil_temp = j.soil_temp;
    if (typeof j.air_hum   === 'number') r.air_hum   = j.air_hum;
    if (typeof j.soil_hum  === 'number') r.soil_hum  = j.soil_hum;
    if (typeof j.light     === 'number') r.light     = j.light;
    if (typeof j.noise     === 'number') r.noise     = j.noise;

    return r;
  } catch {
    return null;
  }
}

// ---- connection flows ----

// 仅显示 P-BIT（新 UUID）的扫描按钮
export async function connectAndStartFiltered() {
  const dev = await navigator.bluetooth.requestDevice({
    filters: [{ services: [NEW_SERVICE_UUID] }],
    optionalServices: [LEGACY_SERVICE_UUID] // 仍允许附带老服务
  });
  return connectInternal(dev, /*mode*/'filtered');
}

// 兼容模式：把能看到的新老设备都列出来（可能会多）
export async function connectAndStartDual() {
  const dev = await navigator.bluetooth.requestDevice({
    // Note: Chrome 允许在同一次 request 中混用 filter + acceptAll
    // 但更稳妥的方式：不用 filters，直接 acceptAll + optionalServices
    acceptAllDevices: true,
    optionalServices: [NEW_SERVICE_UUID, LEGACY_SERVICE_UUID]
  });
  return connectInternal(dev, /*mode*/'compatible');
}

async function connectInternal(dev: BluetoothDevice, mode: string) {
  device = dev;
  sessionStorage.setItem('pbit.deviceName', dev.name || '');
  sessionStorage.setItem('pbit.mode', mode);

  device.addEventListener('gattserverdisconnected', () => {
    // 可选：通知 UI 断开
  });

  server = await dev.gatt!.connect();

  // 新协议
  try {
    const svc = await server.getPrimaryService(NEW_SERVICE_UUID);
    newChar = await svc.getCharacteristic(NEW_CHAR_UUID);
    await newChar.startNotifications();
    newChar.addEventListener('characteristicvaluechanged', (ev: any) => {
      const dv = ev.target.value as DataView;
      const r = parse17(dv);
      if (r) emit(r);
    });

    // 可选：向设备写 0x01 请求即时一包
    await newChar.writeValue(Uint8Array.from([0x01]));
  } catch {
    newChar = null; // 没有新协议也无妨（兼容老固件）
  }

  // 旧协议
  try {
    const svc = await server.getPrimaryService(LEGACY_SERVICE_UUID as BluetoothServiceUUID);
    legacyChar = await svc.getCharacteristic(LEGACY_CHAR_UUID as BluetoothCharacteristicUUID);
    await legacyChar.startNotifications();
    legacyChar.addEventListener('characteristicvaluechanged', (ev: any) => {
      const dv = ev.target.value as DataView;
      const r = parseLegacyJSON(dv);
      if (r) emit(r);
    });
  } catch {
    legacyChar = null;
  }

  return { name: dev.name || 'Unknown', mode };
}

export async function stop() {
  try { await newChar?.stopNotifications(); } catch {}
  try { await legacyChar?.stopNotifications(); } catch {}
  if (server?.connected) {
    try { server.disconnect(); } catch {}
  }
  device = null;
  server = null;
  newChar = null;
  legacyChar = null;
}
