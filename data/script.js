// Inicia conexión WebSocket
let socketConnection = new WebSocket('ws://' + window.location.hostname + ':81/');

// Manejo de mensajes del WebSocket
socketConnection.onmessage = function(event) {
    try {
        const data = JSON.parse(event.data);
        console.log("Datos recibidos:", data); // Verifica los datos en la consola

        // Actualiza los valores de cada sensor y los gráficos
        actualizarSensores(data);
        actualizarGraficos(data);
    } catch (error) {
        console.error("Error al procesar los datos del WebSocket:", error);
    }
};

// Función para enviar comandos de los relés
function turnOn(rele) {
    socketConnection.send(`rele${rele}on`);
}
function turnOff(rele) {
    socketConnection.send(`rele${rele}off`);
}

// Función para actualizar valores en las tarjetas de sensores
function actualizarSensores(data) {
    const sensores = [
        { id: "Luz", value: data.Luz, unit: "lx" },
        { id: "Temperatura", value: data.Temperatura, unit: "C" },
        { id: "Humedad", value: data.Humedad, unit: "%" },
        { id: "Presion", value: data.Presion, unit: "hPa" },
        { id: "Altitud", value: data.Altitud, unit: "m" },
        { id: "pH", value: data.pH, unit: "pH" },
        { id: "TempInt", value: data.TempInt, unit: "C" },
        { id: "tds", value: data.tds, unit: "ppm" }
    ];

    sensores.forEach(sensor => {
        updateCardPercentage(sensor.id, sensor.value, sensor.unit);
    });

    // Actualizar el tiempo de activación de los relés
    document.getElementById('timeRele1').value = data.rele1Time + " segundos";
    document.getElementById('timeRele2').value = data.rele2Time + " segundos";
    document.getElementById('timeRele3').value = data.rele3Time + " segundos";
    document.getElementById('timeRele4').value = data.rele4Time + " segundos";
}

// Función para actualizar el progreso de cada sensor en su tarjeta
function updateCardPercentage(cardId, value, unit) {
    const card = document.getElementById(cardId);
    const text = card.querySelector('.number > h2');
    const circle = card.querySelector('svg circle:nth-child(2)');

    text.innerHTML = `${value}<span>${unit}</span>`;
    const radius = circle.r.baseVal.value;
    const circumference = radius * 2 * Math.PI;
    let offset = circumference - (value / 100) * circumference;

    circle.style.strokeDashoffset = offset;
}

// Función para crear gráficos de forma simplificada
function crearGrafico(ctx, label) {
    return new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: label,
                data: [],
                borderColor: 'rgba(75, 192, 192, 1)',
                borderWidth: 2,
                fill: false,
            }]
        },
        options: {
            responsive: true,
            animation: { duration: 0 },
            scales: {
                x: { display: true },
                y: { display: true, beginAtZero: true }
            }
        }
    });
}

// Crear gráficos para cada sensor en elementos de canvas
const charts = [
    crearGrafico(document.getElementById('myChart1').getContext('2d'), 'Sensor Datos Luz'),
    crearGrafico(document.getElementById('myChart2').getContext('2d'), 'Sensor Datos Temperatura'),
    crearGrafico(document.getElementById('myChart3').getContext('2d'), 'Sensor Datos Humedad'),
    crearGrafico(document.getElementById('myChart4').getContext('2d'), 'Sensor Datos Presion'),
    crearGrafico(document.getElementById('myChart5').getContext('2d'), 'Sensor Datos Altitud'),
    crearGrafico(document.getElementById('myChart6').getContext('2d'), 'Sensor Datos pH'),
    crearGrafico(document.getElementById('myChart7').getContext('2d'), 'Sensor Datos TempInt'),
    crearGrafico(document.getElementById('myChart8').getContext('2d'), 'Sensor Datos TDS')
];

// Función para actualizar gráficos con datos
function actualizarGraficos(data) {
    const valores = [data.Luz, data.Temperatura, data.Humedad, data.Presion, data.Altitud, data.pH, data.TempInt, data.tds];
    const time = new Date().toLocaleTimeString();

    charts.forEach((chart, index) => {
        const valor = valores[index];
        if (typeof valor === "number" && !isNaN(valor)) {  // Asegura que el dato es numérico
            chart.data.labels.push(time);
            chart.data.datasets[0].data.push(valor);

            if (chart.data.labels.length > 50) {
                chart.data.labels.shift();
                chart.data.datasets[0].data.shift();
            }
            chart.update();
        } else {
            console.warn(`Valor no numérico para el gráfico ${index + 1}:`, valor);
        }
    });
}

// Función general para descargar datos en CSV
function descargarCSV(chart, nombreArchivo) {
    const data = chart.data.datasets[0].data;
    if (data.length === 0) return;

    let csvContent = "Time,Value\n";
    chart.data.labels.forEach((label, index) => {
        csvContent += `${label},${data[index]}\n`;
    });

    const blob = new Blob([csvContent], { type: "text/csv;charset=utf-8;" });
    const link = document.createElement("a");
    link.href = URL.createObjectURL(blob);
    link.download = `${nombreArchivo}.csv`;
    link.style.display = "none";
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}

// Asigna eventos a los botones de descarga
for (let i = 0; i < charts.length; i++) {
    document.getElementById(`downloadButton${i + 1}`).addEventListener('click', () => {
        descargarCSV(charts[i], `sensor_data_${i + 1}`);
    });
}

// Manejo de reconexión del WebSocket
socketConnection.onclose = function () {
    console.log("WebSocket cerrado, intentando reconectar...");
    setTimeout(() => {
        socketConnection = new WebSocket('ws://' + window.location.hostname + ':81/');
    }, 1000);
};
socketConnection.onerror = function (error) {
    console.error("Error en WebSocket:", error);
};

// Función para descargar la base de datos
function descargarBaseDatos() {
    // Redirige al usuario al endpoint para descargar la base de datos
    window.location.href = '/download-db';
}



