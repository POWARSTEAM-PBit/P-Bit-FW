import React, { useEffect, useRef, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { subscribe, connectAndStartDual, connectAndStartFiltered } from '../ble';

export default function Landing() {
  const nav = useNavigate();
  const [status, setStatus] = useState('Not connected');
  const jumped = useRef(false);

  // First reading -> go to dashboard
  useEffect(() => {
    const off = subscribe((r) => {
      if (jumped.current) return;
      jumped.current = true;
      sessionStorage.setItem('pbit.lastReading', JSON.stringify(r));
      nav('/dashboard');
    });
    return () => { off(); };
  }, [nav]);

  const connectFiltered = async () => {
    try {
      if (!('bluetooth' in navigator)) {
        setStatus('Web Bluetooth not supported. Use Chrome/Edge over HTTPS or localhost.');
        return;
      }
      setStatus('Connecting (P-BIT only)…');
      const info = await connectAndStartFiltered();
      setStatus(`Connected: ${info.name} (mode: ${info.mode}). Waiting for data…`);
    } catch (e: any) {
      setStatus('Failed: ' + (e?.message || e));
    }
  };

  const connectCompatible = async () => {
    try {
      if (!('bluetooth' in navigator)) {
        setStatus('Web Bluetooth not supported. Use Chrome/Edge over HTTPS or localhost.');
        return;
      }
      setStatus('Connecting (compatible mode)…');
      const info = await connectAndStartDual();
      setStatus(`Connected: ${info.name} (mode: ${info.mode}). Waiting for data…`);
    } catch (e: any) {
      setStatus('Failed: ' + (e?.message || e));
    }
  };

  return (
    <div style={{ padding: 24 }}>
      <h2>P-BIT Connect</h2>
      <div style={{ display: 'flex', gap: 12 }}>
        <button onClick={connectFiltered}>Show P-BIT only (recommended)</button>
        <button onClick={connectCompatible}>Compatible mode (show all)</button>
      </div>
      <p style={{ marginTop: 10 }}>{status}</p>
      <p style={{ fontSize: 12, opacity: 0.7 }}>
        Tip: disconnect other phones/computers from the device before connecting here.
      </p>
    </div>
  );
}
