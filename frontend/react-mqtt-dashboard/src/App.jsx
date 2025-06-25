// Requisitos:
// 1. Crear proyecto con Vite: npm create vite@latest react-mqtt-dashboard --template react
// 2. Instalar dependencias: npm install mqtt recharts leaflet react-leaflet

import React, { useEffect, useState } from "react";
import mqtt from "mqtt";
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend } from "recharts";
import { MapContainer, TileLayer, Marker, Popup } from "react-leaflet";
import "leaflet/dist/leaflet.css";

const MQTT_BROKER = "wss://b6d522ef66224d36a55e598722e7338d.s1.eu.hivemq.cloud:8883/mqtt"; // Reemplaza por tu broker si es diferente
const TOPICS = [
  "sensor/temperatura",
  "sensor/dth11",
  "sensor/gps",
  "sensor/luminosidad"
];

export default function App() {
  const [temperatureData, setTemperatureData] = useState([]);
  const [humidityData, setHumidityData] = useState([]);
  const [lightData, setLightData] = useState([]);
  const [gpsPosition, setGpsPosition] = useState({ lat: 0, lng: 0 });

  useEffect(() => {
    const client = mqtt.connect(MQTT_BROKER);

    client.on("connect", () => {
      console.log("Conectado a MQTT");
      TOPICS.forEach(topic => client.subscribe(topic));
    });

    client.on("message", (topic, message) => {
      const payload = message.toString();
      const time = new Date().toLocaleTimeString();

      if (topic === "sensor/temperatura") {
        setTemperatureData(prev => [...prev.slice(-19), { time, value: parseFloat(payload) }]);
      } else if (topic === "sensor/humedad") {
        setHumidityData(prev => [...prev.slice(-19), { time, value: parseFloat(payload) }]);
      } else if (topic === "sensor/luz") {
        setLightData(prev => [...prev.slice(-19), { time, value: parseFloat(payload) }]);
      } else if (topic === "sensor/gps") {
        try {
          const [lat, lng] = payload.split(",").map(Number);
          setGpsPosition({ lat, lng });
        } catch (err) {
          console.error("Error al parsear GPS", err);
        }
      }
    });

    return () => client.end();
  }, []);

  return (
    <div className="p-6 space-y-6">
      <h1 className="text-2xl font-bold">Dashboard de Sensores MQTT</h1>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <SensorChart title="Temperatura (°C)" data={temperatureData} />
        <SensorChart title="Humedad (%)" data={humidityData} />
        <SensorChart title="Luz (Lux)" data={lightData} />

        <div>
          <h2 className="text-xl font-semibold mb-2">Ubicación GPS</h2>
          <MapContainer center={[gpsPosition.lat, gpsPosition.lng]} zoom={13} style={{ height: "300px", width: "100%" }}>
            <TileLayer url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png" />
            <Marker position={[gpsPosition.lat, gpsPosition.lng]}>
              <Popup>
                Lat: {gpsPosition.lat} <br /> Lng: {gpsPosition.lng}
              </Popup>
            </Marker>
          </MapContainer>
        </div>
      </div>
    </div>
  );
}

function SensorChart({ title, data }) {
  return (
    <div>
      <h2 className="text-xl font-semibold mb-2">{title}</h2>
      <LineChart
        width={500}
        height={250}
        data={data}
        margin={{ top: 5, right: 20, left: 10, bottom: 5 }}
      >
        <CartesianGrid strokeDasharray="3 3" />
        <XAxis dataKey="time" />
        <YAxis />
        <Tooltip />
        <Legend />
        <Line type="monotone" dataKey="value" stroke="#8884d8" strokeWidth={2} />
      </LineChart>
    </div>
  );
}
