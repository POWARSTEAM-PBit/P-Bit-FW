import React, { useEffect, useState } from 'react';
import { subscribe, stop, type Reading } from '../ble';

type View = {
  temp?: number;
  hum?: number;
  ldr?: number;
  mic?: number;
  batt?: number;
  note?: string; // indicates which legacy fields were mapped
};

function mapToView(r: Reading): View {
  const v: View = {};
  v.temp = r.temp ?? r.air_temp ?? r.soil_temp;
  v.hum  = r.hum  ?? r.air_hum  ?? r.soil_hum;
  v.ldr  = r.ldr  ?? r.light;
  v.mic  = r.mic  ?? r.noise;
  v.batt = r.batt;

  const used: string[] = [];
  if (r.air_temp  && v.temp === r.air_temp)  used.push('air_temp');
  if (r.soil_temp && v.temp === r.soil_temp) used.push('soil_temp');
  if (r.air_hum   && v.hum  === r.air_hum)   used.push('air_hum');
  if (r.soil_hum  && v.hum  === r.soil_hum)  used.push('soil_hum');
  if (r.light     && v.ldr  === r.light)     used.push('light');
  if (r.noise     && v.mic  === r.noise)     used.push('noise');
  if (used.length) v.note = `legacy mapping: ${used.join(', ')}`;
  return v;
}

export default function Dashboard() {
  const [raw, setRaw] = useState<Reading | null>(null);
  const [view, setView] = useState<View>({});
  const [mode, setMode] = useState<string>(() => sessionStorage.getItem('pbit.mode') || '');
  const [devName, setDevName] = useState<string>(() => sessionStorage.getItem('pbit.deviceName') || '');

  useEffect(() => {
    const off = subscribe((data) => {
      setRaw(data);
      setView(mapToView(data));
      const m = sessionStorage.getItem('pbit.mode'); if (m) setMode(m);
      const n = sessionStorage.getItem('pbit.deviceName'); if (n) setDevName(n);
    });
    return () => { off(); };
  }, []);

  return (
    <div style={{ padding: 24 }}>
      <h2>P-BIT Dashboard</h2>
      <div style={{ opacity: 0.7, marginBottom: 8 }}>
        Device: {devName || 'Unknown'} Mode: {mode || '—'}
      </div>

      {raw ? (
        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(3, 220px)', gap: 12 }}>
          <Card label="Temperature (°C)" value={view.temp} />
          <Card label="Humidity (%)" value={view.hum} />
          <Card label="Light" value={view.ldr} />
          <Card label="Mic/Noise" value={view.mic} />
          <Card label="Battery / Voltage" value={view.batt} />
          <Card label="Timestamp" value={new Date(raw.ts).toLocaleTimeString()} />
        </div>
      ) : (
        <p>Waiting for data… (ensure the device isn’t connected elsewhere)</p>
      )}

      {view.note && <p style={{ marginTop: 8, fontSize: 12, color: '#888' }}>{view.note}</p>}

      <div style={{ marginTop: 16 }}>
        <button onClick={() => { stop(); window.location.href = '/'; }}>Disconnect & Back</button>
      </div>
    </div>
  );
}

function Card({ label, value }: { label: string; value: any }) {
  const display = value == null ? '—' :
    (typeof value === 'number' ? (Math.round(value * 100) / 100).toString() : String(value));
  return (
    <div style={{ border: '1px solid #ddd', borderRadius: 8, padding: 12 }}>
      <div style={{ fontSize: 12, opacity: 0.7 }}>{label}</div>
      <div style={{ fontSize: 20, marginTop: 4 }}>{display}</div>
    </div>
  );
}
