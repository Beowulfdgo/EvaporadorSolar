import React, { useState, useEffect, useRef } from 'react';
import { Thermometer, Droplets, Navigation, Sun, Scale, RotateCw, MapPin, Wifi, WifiOff, Power, PowerOff } from 'lucide-react';
import mqtt from 'mqtt';
const MQTTDashboard = () => {
  const [connectionStatus, setConnectionStatus] = useState('disconnected');
  const [sensorData, setSensorData] = useState({
    dht11: { temperatura: 0, humedad: 0, fecha: '', hora: '' },
    giroscopio: { latitud: 0, longitud: 0, fecha: '', hora: '' },
    luminosidad: { lux: 0, fecha: '', hora: '' },
    bascula: { peso_mg: 0, fecha: '', hora: '' },
    gps: { latitud: 19.4326, longitud: -99.1332, fecha: '', hora: '' },
    motorX: { estatus: 'OFF', grados: 0, fecha: '', hora: '' },
    motorY: { estatus: 'OFF', grados: 0, fecha: '', hora: '' }
  });

  const clientRef = useRef(null);

  // Configuración MQTT para HiveMQ Cloud
  const mqttConfig = {
    host: 'b6d522ef66224d36a55e598722e7338d.s1.eu.hivemq.cloud',
    port: 8884,
    protocol: 'wss',
    username: 'evaporador',
    password: 'Evaporador2025'
  };

  // CSS-in-JS styles
  const styles = {
    container: {
      minHeight: '100vh',
      background: 'linear-gradient(135deg, #f0f9ff 0%, #e0e7ff 100%)',
      padding: '20px',
      fontFamily: 'Arial, sans-serif'
    },
    maxWidth: {
      maxWidth: '1200px',
      margin: '0 auto'
    },
    header: {
      background: 'white',
      borderRadius: '12px',
      boxShadow: '0 4px 6px rgba(0, 0, 0, 0.1)',
      padding: '24px',
      marginBottom: '24px',
      display: 'flex',
      justifyContent: 'space-between',
      alignItems: 'center',
      flexWrap: 'wrap'
    },
    title: {
      margin: 0,
      fontSize: '2rem',
      color: '#1f2937',
      fontWeight: 'bold'
    },
    subtitle: {
      margin: '4px 0 0 0',
      color: '#6b7280',
      fontSize: '1rem'
    },
    connectionStatus: {
      display: 'flex',
      alignItems: 'center',
      gap: '8px'
    },
    statusBadge: {
      padding: '4px 12px',
      borderRadius: '20px',
      fontSize: '0.875rem',
      fontWeight: '500'
    },
    statusConnected: {
      backgroundColor: '#dcfce7',
      color: '#166534'
    },
    statusConnecting: {
      backgroundColor: '#fef3c7',
      color: '#92400e'
    },
    statusDisconnected: {
      backgroundColor: '#fee2e2',
      color: '#991b1b'
    },
    grid: {
      display: 'grid',
      gridTemplateColumns: 'repeat(auto-fit, minmax(300px, 1fr))',
      gap: '24px',
      marginBottom: '24px'
    },
    card: {
      background: 'white',
      borderRadius: '12px',
      boxShadow: '0 4px 6px rgba(0, 0, 0, 0.1)',
      padding: '24px',
      borderLeft: '4px solid #3b82f6'
    },
    cardWarning: {
      borderLeftColor: '#f59e0b'
    },
    cardDanger: {
      borderLeftColor: '#ef4444'
    },
    cardHeader: {
      display: 'flex',
      justifyContent: 'space-between',
      alignItems: 'center',
      marginBottom: '16px'
    },
    cardTitle: {
      margin: 0,
      fontSize: '1.125rem',
      fontWeight: '600',
      color: '#1f2937'
    },
    dataRow: {
      display: 'flex',
      justifyContent: 'space-between',
      alignItems: 'center',
      marginBottom: '12px'
    },
    label: {
      fontSize: '0.875rem',
      color: '#6b7280'
    },
    value: {
      fontFamily: 'monospace',
      fontSize: '1.25rem',
      fontWeight: 'bold'
    },
    valueLarge: {
      fontSize: '1.5rem'
    },
    valueTemp: {
      color: '#ef4444'
    },
    valueHumidity: {
      color: '#3b82f6'
    },
    valueLux: {
      color: '#f59e0b'
    },
    valueWeight: {
      color: '#8b5cf6'
    },
    timestamp: {
      fontSize: '0.75rem',
      color: '#9ca3af',
      marginTop: '8px'
    },
    motorControls: {
      display: 'flex',
      flexDirection: 'column',
      gap: '16px'
    },
    motorStatus: {
      display: 'flex',
      alignItems: 'center',
      gap: '8px',
      padding: '8px 12px',
      borderRadius: '20px',
      fontSize: '0.875rem',
      fontWeight: '500'
    },
    motorStatusOn: {
      backgroundColor: '#dcfce7',
      color: '#166534'
    },
    motorStatusOff: {
      backgroundColor: '#fee2e2',
      color: '#991b1b'
    },
    buttonRow: {
      display: 'flex',
      gap: '8px'
    },
    button: {
      flex: 1,
      padding: '8px 16px',
      border: 'none',
      borderRadius: '8px',
      fontSize: '0.875rem',
      fontWeight: '500',
      cursor: 'pointer',
      transition: 'background-color 0.2s'
    },
    buttonGreen: {
      backgroundColor: '#10b981',
      color: 'white'
    },
    buttonGreenHover: {
      backgroundColor: '#059669'
    },
    buttonRed: {
      backgroundColor: '#ef4444',
      color: 'white'
    },
    buttonRedHover: {
      backgroundColor: '#dc2626'
    },
    buttonBlue: {
      backgroundColor: '#3b82f6',
      color: 'white'
    },
    buttonBlueHover: {
      backgroundColor: '#2563eb'
    },
    input: {
      flex: 1,
      padding: '8px 12px',
      border: '1px solid #d1d5db',
      borderRadius: '8px',
      fontSize: '0.875rem'
    },
    instructions: {
      background: 'white',
      borderRadius: '12px',
      boxShadow: '0 4px 6px rgba(0, 0, 0, 0.1)',
      padding: '24px'
    },
    instructionsGrid: {
      display: 'grid',
      gridTemplateColumns: 'repeat(auto-fit, minmax(250px, 1fr))',
      gap: '24px',
      marginTop: '16px'
    },
    codeBlock: {
      backgroundColor: '#f3f4f6',
      padding: '8px',
      borderRadius: '4px',
      fontFamily: 'monospace',
      fontSize: '0.875rem'
    },
    alertBox: {
      marginTop: '24px',
      padding: '16px',
      backgroundColor: '#dbeafe',
      borderRadius: '8px'
    }
  };

  // Simular conexión MQTT
  useEffect(() => {
    const connectToMQTT = () => {
      setConnectionStatus('connecting');
      
      setTimeout(() => {
        setConnectionStatus('connected');
        console.log('Conectado a HiveMQ Cloud');
        startDataSimulation();
      }, 2000);
    };

    connectToMQTT();

    return () => {
      if (clientRef.current) {
        setConnectionStatus('disconnected');
      }
    };
  }, []);

  // Simulación de datos entrantes
  const startDataSimulation = () => {
    const interval = setInterval(() => {
      const newDHT11 = {
        temperatura: (Math.random() * (35 - 20) + 20).toFixed(1),
        humedad: (Math.random() * (70 - 30) + 30).toFixed(1),
        fecha: new Date().toISOString().split('T')[0],
        hora: new Date().toTimeString().split(' ')[0]
      };

      const newGiroscopio = {
        latitud: (Math.random() * 180 - 90).toFixed(2),
        longitud: (Math.random() * 360 - 180).toFixed(2),
        fecha: new Date().toISOString().split('T')[0],
        hora: new Date().toTimeString().split(' ')[0]
      };

      const newLuminosidad = {
        lux: Math.floor(Math.random() * 10000),
        fecha: new Date().toISOString().split('T')[0],
        hora: new Date().toTimeString().split(' ')[0]
      };

      const newBascula = {
        peso_mg: Math.floor(Math.random() * (500000 - 50000) + 50000),
        fecha: new Date().toISOString().split('T')[0],
        hora: new Date().toTimeString().split(' ')[0]
      };

      setSensorData(prev => {
        const newGPS = {
          latitud: prev.gps.latitud + (Math.random() - 0.5) * 0.001,
          longitud: prev.gps.longitud + (Math.random() - 0.5) * 0.001,
          fecha: new Date().toISOString().split('T')[0],
          hora: new Date().toTimeString().split(' ')[0]
        };

        return {
          ...prev,
          dht11: newDHT11,
          giroscopio: newGiroscopio,
          luminosidad: newLuminosidad,
          bascula: newBascula,
          gps: newGPS
        };
      });
    }, 3000);

    return () => clearInterval(interval);
  };

  // Función para enviar comandos a motores
  const sendMotorCommand = (motor, command) => {
    console.log(`Enviando comando ${command} al ${motor}`);
    
    if (clientRef.current && connectionStatus === 'connected') {
      console.log(`Comando MQTT enviado: evaporador/${motor}/cmd -> ${command}`);
    }
    
    setSensorData(prev => ({
      ...prev,
      [motor]: {
        ...prev[motor],
        estatus: command === 'ON' || command === 'OFF' ? command : prev[motor].estatus,
        grados: command.startsWith('GRADOS:') ? parseInt(command.split(':')[1]) || prev[motor].grados : prev[motor].grados,
        fecha: new Date().toISOString().split('T')[0],
        hora: new Date().toTimeString().split(' ')[0]
      }
    }));
  };

  const SensorCard = ({ title, icon: Icon, children, status = 'normal' }) => {
    let cardStyle = { ...styles.card };
    if (status === 'warning') cardStyle = { ...cardStyle, ...styles.cardWarning };
    if (status === 'danger') cardStyle = { ...cardStyle, ...styles.cardDanger };

    return (
      <div style={cardStyle}>
        <div style={styles.cardHeader}>
          <h3 style={styles.cardTitle}>{title}</h3>
          <Icon size={24} color={status === 'warning' ? '#f59e0b' : status === 'danger' ? '#ef4444' : '#3b82f6'} />
        </div>
        {children}
      </div>
    );
  };

  const MotorControl = ({ motor, data }) => (
    <div style={styles.motorControls}>
      <div style={styles.dataRow}>
        <span style={styles.label}>Estado:</span>
        <div style={{
          ...styles.motorStatus,
          ...(data.estatus === 'ON' ? styles.motorStatusOn : styles.motorStatusOff)
        }}>
          {data.estatus === 'ON' ? <Power size={16} /> : <PowerOff size={16} />}
          {data.estatus}
        </div>
      </div>
      
      <div style={styles.dataRow}>
        <span style={styles.label}>Grados:</span>
        <span style={{...styles.value, ...styles.valueLarge}}>{data.grados}°</span>
      </div>
      
      <div style={styles.buttonRow}>
        <button
          style={{...styles.button, ...styles.buttonGreen}}
          onClick={() => sendMotorCommand(motor, 'ON')}
          onMouseOver={(e) => e.target.style.backgroundColor = '#059669'}
          onMouseOut={(e) => e.target.style.backgroundColor = '#10b981'}
        >
          ON
        </button>
        <button
          style={{...styles.button, ...styles.buttonRed}}
          onClick={() => sendMotorCommand(motor, 'OFF')}
          onMouseOver={(e) => e.target.style.backgroundColor = '#dc2626'}
          onMouseOut={(e) => e.target.style.backgroundColor = '#ef4444'}
        >
          OFF
        </button>
      </div>
      
      <div style={styles.buttonRow}>
        <input
          type="number"
          placeholder="Grados"
          style={styles.input}
          onKeyPress={(e) => {
            if (e.key === 'Enter') {
              sendMotorCommand(motor, `GRADOS:${e.target.value}`);
              e.target.value = '';
            }
          }}
        />
        <button
          style={{...styles.button, ...styles.buttonBlue}}
          onClick={(e) => {
            const input = e.target.previousElementSibling;
            if (input.value) {
              sendMotorCommand(motor, `GRADOS:${input.value}`);
              input.value = '';
            }
          }}
          onMouseOver={(e) => e.target.style.backgroundColor = '#2563eb'}
          onMouseOut={(e) => e.target.style.backgroundColor = '#3b82f6'}
        >
          Set
        </button>
      </div>
      
      <div style={styles.timestamp}>
        Última actualización: {data.fecha} {data.hora}
      </div>
    </div>
  );

  return (
    <div style={styles.container}>
      <div style={styles.maxWidth}>
        {/* Header */}
        <div style={styles.header}>
          <div>
            <h1 style={styles.title}>Dashboard ESP32</h1>
            <p style={styles.subtitle}>Monitoreo en tiempo real - HiveMQ Cloud</p>
          </div>
          <div style={styles.connectionStatus}>
            {connectionStatus === 'connected' ? (
              <Wifi size={24} color="#10b981" />
            ) : (
              <WifiOff size={24} color="#ef4444" />
            )}
            <span style={{
              ...styles.statusBadge,
              ...(connectionStatus === 'connected' ? styles.statusConnected :
                 connectionStatus === 'connecting' ? styles.statusConnecting :
                 styles.statusDisconnected)
            }}>
              {connectionStatus === 'connected' ? 'Conectado' :
               connectionStatus === 'connecting' ? 'Conectando...' : 'Desconectado'}
            </span>
          </div>
        </div>

        {/* Grid de sensores */}
        <div style={styles.grid}>
          {/* DHT11 - Temperatura y Humedad */}
          <SensorCard 
            title="DHT11 - Clima" 
            icon={Thermometer}
            status={parseFloat(sensorData.dht11.temperatura) > 30 ? 'warning' : 'normal'}
          >
            <div style={styles.dataRow}>
              <span style={styles.label}>Temperatura:</span>
              <span style={{...styles.value, ...styles.valueLarge, ...styles.valueTemp}}>
                {sensorData.dht11.temperatura}°C
              </span>
            </div>
            <div style={styles.dataRow}>
              <span style={styles.label}>Humedad:</span>
              <span style={{...styles.value, ...styles.valueLarge, ...styles.valueHumidity}}>
                {sensorData.dht11.humedad}%
              </span>
            </div>
            <div style={styles.timestamp}>
              {sensorData.dht11.fecha} {sensorData.dht11.hora}
            </div>
          </SensorCard>

          {/* Giroscopio */}
          <SensorCard title="Giroscopio" icon={Navigation}>
            <div style={styles.dataRow}>
              <span style={styles.label}>Latitud:</span>
              <span style={styles.value}>{sensorData.giroscopio.latitud}°</span>
            </div>
            <div style={styles.dataRow}>
              <span style={styles.label}>Longitud:</span>
              <span style={styles.value}>{sensorData.giroscopio.longitud}°</span>
            </div>
            <div style={styles.timestamp}>
              {sensorData.giroscopio.fecha} {sensorData.giroscopio.hora}
            </div>
          </SensorCard>

          {/* Luminosidad */}
          <SensorCard title="Luminosidad" icon={Sun}>
            <div style={styles.dataRow}>
              <span style={styles.label}>Lux:</span>
              <span style={{...styles.value, ...styles.valueLarge, ...styles.valueLux}}>
                {sensorData.luminosidad.lux}
              </span>
            </div>
            <div style={styles.timestamp}>
              {sensorData.luminosidad.fecha} {sensorData.luminosidad.hora}
            </div>
          </SensorCard>

          {/* Báscula */}
          <SensorCard title="Báscula" icon={Scale}>
            <div style={styles.dataRow}>
              <span style={styles.label}>Peso:</span>
              <span style={{...styles.value, ...styles.valueWeight}}>
                {(sensorData.bascula.peso_mg / 1000).toFixed(1)}g
              </span>
            </div>
            <div style={styles.dataRow}>
              <span style={styles.label}>Mg:</span>
              <span style={{...styles.value, fontSize: '0.875rem', color: '#6b7280'}}>
                {sensorData.bascula.peso_mg}
              </span>
            </div>
            <div style={styles.timestamp}>
              {sensorData.bascula.fecha} {sensorData.bascula.hora}
            </div>
          </SensorCard>

          {/* GPS */}
          <SensorCard title="GPS" icon={MapPin}>
            <div style={styles.dataRow}>
              <span style={styles.label}>Latitud:</span>
              <span style={{...styles.value, fontSize: '0.875rem'}}>{sensorData.gps.latitud.toFixed(6)}°</span>
            </div>
            <div style={styles.dataRow}>
              <span style={styles.label}>Longitud:</span>
              <span style={{...styles.value, fontSize: '0.875rem'}}>{sensorData.gps.longitud.toFixed(6)}°</span>
            </div>
            <div style={styles.timestamp}>
              {sensorData.gps.fecha} {sensorData.gps.hora}
            </div>
          </SensorCard>

          {/* Motor X */}
          <SensorCard title="Motor X" icon={RotateCw}>
            <MotorControl motor="motorX" data={sensorData.motorX} />
          </SensorCard>

          {/* Motor Y */}
          <SensorCard title="Motor Y" icon={RotateCw}>
            <MotorControl motor="motorY" data={sensorData.motorY} />
          </SensorCard>

          {/* Configuración MQTT */}
          <SensorCard title="Configuración MQTT" icon={Wifi}>
            <div style={{fontSize: '0.875rem', lineHeight: '1.5'}}>
              <div><strong>Host:</strong> {mqttConfig.host}</div>
              <div><strong>Puerto:</strong> {mqttConfig.port}</div>
              <div><strong>Protocolo:</strong> {mqttConfig.protocol}</div>
              <div><strong>Usuario:</strong> {mqttConfig.username}</div>
              <div style={{...styles.timestamp, marginTop: '12px'}}>
                Para usar MQTT real, instala: npm install mqtt
              </div>
            </div>
          </SensorCard>
        </div>

        {/* Instrucciones */}
        <div style={styles.instructions}>
          <h2 style={{...styles.cardTitle, fontSize: '1.25rem', marginBottom: '16px'}}>
            Instrucciones de Implementación MQTT Real
          </h2>
          <div style={styles.instructionsGrid}>
            <div>
              <h3 style={{...styles.cardTitle, fontSize: '1rem', marginBottom: '8px'}}>1. Instalar dependencias:</h3>
              <div style={styles.codeBlock}>npm install mqtt</div>
              <p style={{fontSize: '0.875rem', marginTop: '8px'}}>
                Luego importar: <code>import mqtt from 'mqtt';</code>
              </p>
            </div>
            <div>
              <h3 style={{...styles.cardTitle, fontSize: '1rem', marginBottom: '8px'}}>2. Topics suscritos:</h3>
              <ul style={{fontSize: '0.875rem', paddingLeft: '20px'}}>
                <li>evaporador/dht11/json</li>
                <li>evaporador/giroscopio/json</li>
                <li>evaporador/luminosidad/json</li>
                <li>evaporador/bascula/json</li>
                <li>evaporador/gps/json</li>
                <li>evaporador/motorX/json</li>
                <li>evaporador/motorY/json</li>
              </ul>
            </div>
            <div>
              <h3 style={{...styles.cardTitle, fontSize: '1rem', marginBottom: '8px'}}>3. Topics de comando:</h3>
              <ul style={{fontSize: '0.875rem', paddingLeft: '20px'}}>
                <li>evaporador/motorX/cmd</li>
                <li>evaporador/motorY/cmd</li>
              </ul>
              <p style={{fontSize: '0.875rem', marginTop: '8px'}}>
                Comandos: ON, OFF, GRADOS:120
              </p>
            </div>
            <div>
              <h3 style={{...styles.cardTitle, fontSize: '1rem', marginBottom: '8px'}}>4. Ejemplo de conexión:</h3>
              <div style={styles.codeBlock}>
                <pre style={{margin: 0, fontSize: '0.75rem'}}>
{`const client = mqtt.connect('wss://...', {
  username: 'evaporador',
  password: 'Evaporador2025'
});

clientRef.current = client;`}
                </pre>
              </div>
            </div>
          </div>
          
          <div style={styles.alertBox}>
            <h3 style={{...styles.cardTitle, fontSize: '1rem', color: '#1e40af', marginBottom: '8px'}}>
              Estado actual:
            </h3>
            <p style={{color: '#1e40af', margin: 0}}>
              Dashboard ejecutándose en modo simulación. Los controles de motor actualizan el estado local.
            </p>
          </div>
        </div>
      </div>
    </div>
  );
};

export default MQTTDashboard;