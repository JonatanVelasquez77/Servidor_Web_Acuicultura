let socketConnection = new WebSocket('ws://' + window.location.hostname + ':81/');

socketConnection.onmessage = function (event) {
	var data = JSON.parse(event.data);

	updateCardPercentage("Luz", data.Luz, "lx");
	updateCardPercentage("Temperatura", data.Temperatura, "C");
	updateCardPercentage("Humedad", data.Humedad, "%");
	updateCardPercentage("Presion", data.Presion, "hPa");
	updateCardPercentage("Altitud", data.Altitud, "m");
	//updateCardPercentage("pH", data.pH, "pH");
	//updateCardPercentage("Temperatura_Interna", data.Temperatura_Interna, "C");
	//updateCardPercentage("Oxigeno", data.Oxigeno, "m");

	var time = new Date().toLocaleTimeString();
  	var value = data.Luz; // Puedes ajustar esto según el valor que quieras graficar
	var value2 = data.Temperatura;
	var value3 = data.Humedad;
	var value4 = data.Presion;
	var value5 = data.Altitud;
  	addData(time, value,value2,value3,value4,value5);
};

function updateCardPercentage(cardId, value, unit) {
	const card = document.getElementById(cardId);

	const text = card.querySelector('.number>h2');
	const circle = card.querySelector('svg circle:nth-child(2)');

	text.innerHTML = `${value}<span>${unit}</span>`;

	const radius = circle.r.baseVal.value;
	const circumference = radius * 2 * Math.PI;

	if (cardId == "Luz") {
	  const offset = circumference - (value / 2000) * circumference;
	  circle.style.strokeDashoffset = offset;

	  if (value >= 80) {
		circle.style.stroke = '#00ff43';
	  } else if (value >= 40) {
		circle.style.stroke = '#41c2c3';
	  } else {
		circle.style.stroke = '#ff04f7';
	  }
	} else if (cardId == "Temperatura") {
	  const offset = circumference - (value / 60) * circumference;
	  circle.style.strokeDashoffset = offset;
	  circle.style.stroke = '#00a1ff';
	} else if (cardId == "Humedad") {
	  const offset = circumference - (value / 100) * circumference;
	  circle.style.strokeDashoffset = offset;
	  circle.style.stroke = '#ec4e0d';
	} else if (cardId == "Presion") {
		const offset = circumference - (value / 1200) * circumference;
		circle.style.strokeDashoffset = offset;
		circle.style.stroke = '#ec4e0d';
	} else if (cardId == "Altitud") {
		const offset = circumference - (value / 5) * circumference;
		circle.style.strokeDashoffset = offset;
		circle.style.stroke = '#ec4e0d';
	}

	card.setAttribute('data-value', value);
}

function getRandomPercentage() {
	return Math.floor(Math.random() * 101);
}

const turnOn = () => socketConnection.send("LedOn");
const turnOff = () => socketConnection.send("LedOff");

var ctx1 = document.getElementById('myChart1').getContext('2d');

var myChart1 = new Chart(ctx1, {
	type: 'line',
	data: {
	  labels: [],
	  datasets: [{
		label: 'Sensor Datos Luz',
		data: [],
		borderColor: 'rgba(75, 192, 192, 1)',
		borderWidth: 2,
		fill: false,
	  }]
	},
	options: {
		animation: {
			duration: 0, // Desactiva la animación o ajusta la duración según tus preferencias
		  },
		scales: {
			x: [{
			type: 'linear',
			position: 'bottom'
			}],
			y: [{
			type: 'linear',
			position: 'left',
			ticks: {
				min: 0, // Valor mínimo del eje Y
				max: 100, // Valor máximo del eje Y
				stepSize: 10, // Incremento entre los valores del eje Y
			}
			}]
		}
	}
});

var ctx2 = document.getElementById('myChart2').getContext('2d');

var myChart2 = new Chart(ctx2, {
	type: 'line',
	data: {
	  labels: [],
	  datasets: [{
		label: 'Sensor Datos Temperatura',
		data: [],
		borderColor: 'rgba(75, 192, 192, 1)',
		borderWidth: 2,
		fill: false,
	  }]
	},
	options: {
		animation: {
			duration: 0, // Desactiva la animación o ajusta la duración según tus preferencias
		  },
		scales: {
			x: [{
			type: 'linear',
			position: 'bottom'
			}],
			y: [{
			type: 'linear',
			position: 'left',
			ticks: {
				min: 0, // Valor mínimo del eje Y
				max: 100, // Valor máximo del eje Y
				stepSize: 10, // Incremento entre los valores del eje Y
			}
			}]
		}
	}
});

var ctx3 = document.getElementById('myChart3').getContext('2d');

var myChart3 = new Chart(ctx3, {
	type: 'line',
	data: {
	  labels: [],
	  datasets: [{
		label: 'Sensor Datos Humedad',
		data: [],
		borderColor: 'rgba(75, 192, 192, 1)',
		borderWidth: 2,
		fill: false,
	  }]
	},
	options: {
		animation: {
			duration: 0, // Desactiva la animación o ajusta la duración según tus preferencias
		  },
		scales: {
			x: [{
			type: 'linear',
			position: 'bottom'
			}],
			y: [{
			type: 'linear',
			position: 'left',
			ticks: {
				min: 0, // Valor mínimo del eje Y
				max: 100, // Valor máximo del eje Y
				stepSize: 10, // Incremento entre los valores del eje Y
			}
			}]
		}
	}
});

var ctx4 = document.getElementById('myChart4').getContext('2d');

var myChart4 = new Chart(ctx4, {
	type: 'line',
	data: {
	  labels: [],
	  datasets: [{
		label: 'Sensor Datos Presion',
		data: [],
		borderColor: 'rgba(75, 192, 192, 1)',
		borderWidth: 2,
		fill: false,
	  }]
	},
	options: {
		animation: {
			duration: 0, // Desactiva la animación o ajusta la duración según tus preferencias
		  },
		scales: {
			x: [{
			type: 'linear',
			position: 'bottom'
			}],
			y: [{
			type: 'linear',
			position: 'left',
			ticks: {
				min: 0, // Valor mínimo del eje Y
				max: 100, // Valor máximo del eje Y
				stepSize: 10, // Incremento entre los valores del eje Y
			}
			}]
		}
	}
});

var ctx5 = document.getElementById('myChart5').getContext('2d');

var myChart5 = new Chart(ctx5, {
	type: 'line',
	data: {
	  labels: [],
	  datasets: [{
		label: 'Sensor Datos Altitud',
		data: [],
		borderColor: 'rgba(75, 192, 192, 1)',
		borderWidth: 2,
		fill: false,
	  }]
	},
	options: {
		animation: {
			duration: 0, // Desactiva la animación o ajusta la duración según tus preferencias
		  },
		scales: {
			x: [{
			type: 'linear',
			position: 'bottom'
			}],
			y: [{
			type: 'linear',
			position: 'left',
			ticks: {
				min: 0, // Valor mínimo del eje Y
				max: 100, // Valor máximo del eje Y
				stepSize: 10, // Incremento entre los valores del eje Y
			}
			}]
		}
	}
});

var ctx6 = document.getElementById('myChart6').getContext('2d');

var myChart6 = new Chart(ctx6, {
	type: 'line',
	data: {
	  labels: [],
	  datasets: [{
		label: 'Sensor Datos pH',
		data: [],
		borderColor: 'rgba(75, 192, 192, 1)',
		borderWidth: 2,
		fill: false,
	  }]
	},
	options: {
		animation: {
			duration: 0, // Desactiva la animación o ajusta la duración según tus preferencias
		  },
		scales: {
			x: [{
			type: 'linear',
			position: 'bottom'
			}],
			y: [{
			type: 'linear',
			position: 'left',
			ticks: {
				min: 0, // Valor mínimo del eje Y
				max: 100, // Valor máximo del eje Y
				stepSize: 10, // Incremento entre los valores del eje Y
			}
			}]
		}
	}
});

var ctx7 = document.getElementById('myChart7').getContext('2d');

var myChart7 = new Chart(ctx7, {
	type: 'line',
	data: {
	  labels: [],
	  datasets: [{
		label: 'Sensor Datos Temperatura Interna',
		data: [],
		borderColor: 'rgba(75, 192, 192, 1)',
		borderWidth: 2,
		fill: false,
	  }]
	},
	options: {
		animation: {
			duration: 0, // Desactiva la animación o ajusta la duración según tus preferencias
		  },
		scales: {
			x: [{
			type: 'linear',
			position: 'bottom'
			}],
			y: [{
			type: 'linear',
			position: 'left',
			ticks: {
				min: 0, // Valor mínimo del eje Y
				max: 100, // Valor máximo del eje Y
				stepSize: 10, // Incremento entre los valores del eje Y
			}
			}]
		}
	}
});

var ctx8 = document.getElementById('myChart8').getContext('2d');

var myChart8 = new Chart(ctx8, {
	type: 'line',
	data: {
	  labels: [],
	  datasets: [{
		label: 'Sensor Datos Oxigeno',
		data: [],
		borderColor: 'rgba(75, 192, 192, 1)',
		borderWidth: 2,
		fill: false,
	  }]
	},
	options: {
		animation: {
			duration: 0, // Desactiva la animación o ajusta la duración según tus preferencias
		  },
		scales: {
			x: [{
			type: 'linear',
			position: 'bottom'
			}],
			y: [{
			type: 'linear',
			position: 'left',
			ticks: {
				min: 0, // Valor mínimo del eje Y
				max: 100, // Valor máximo del eje Y
				stepSize: 10, // Incremento entre los valores del eje Y
			}
			}]
		}
	}
});

function addData(time, value,value2,value3,value4,value5) {
    myChart.data.labels.push(time);
    myChart.data.datasets[0].data.push(value);
	if (myChart.data.labels.length > 50) {
        myChart.data.labels.shift();
        myChart.data.datasets[0].data.shift();
    }

	myChart2.data.labels.push(time);
    myChart2.data.datasets[0].data.push(value2);
	if (myChart2.data.labels.length > 50) {
        myChart2.data.labels.shift();
        myChart2.data.datasets[0].data.shift();
    }

	myChart3.data.labels.push(time);
    myChart3.data.datasets[0].data.push(value3);
	if (myChart3.data.labels.length > 50) {
        myChart3.data.labels.shift();
        myChart3.data.datasets[0].data.shift();
    }

	myChart4.data.labels.push(time);
    myChart4.data.datasets[0].data.push(value4);
	if (myChart4.data.labels.length > 50) {
        myChart4.data.labels.shift();
        myChart4.data.datasets[0].data.shift();
    }

	myChart5.data.labels.push(time);
    myChart5.data.datasets[0].data.push(value5);
	if (myChart5.data.labels.length > 50) {
        myChart5.data.labels.shift();
        myChart5.data.datasets[0].data.shift();
    }

    myChart.update();
	myChart2.update();
	myChart3.update();
	myChart4.update();
	myChart5.update();
	
}

// Función para crear enlace de descarga si no existe
function createDownloadLink(index) {
    var downloadLink = document.getElementById('downloadLink' + index);
    if (!downloadLink) {
        downloadLink = document.createElement("a");
        downloadLink.id = 'downloadLink' + index;
        document.body.appendChild(downloadLink);
    }
    return downloadLink;
}

// Función para descargar CSV para un gráfico específico
function downloadCSV(chartIndex) {
    var chart = window['myChart' + chartIndex];
    var data = chart.data.datasets[0].data;

    if (data.length > 0) {
        var csvContent = "Time,Value\n";

        data.forEach(function (value, index) {
            csvContent += chart.data.labels[index] + "," + value + "\n";
        });

        var blob = new Blob([csvContent], { type: "text/csv;charset=utf-8;" });

        var downloadLink = createDownloadLink(chartIndex);
        // Utiliza el enlace correspondiente para la descarga
        downloadLink.href = window.URL.createObjectURL(blob);
        downloadLink.setAttribute("download", "sensor_data_" + chartIndex + ".csv");
        downloadLink.style.visibility = "hidden";

        // Simula un clic en el enlace para iniciar la descarga
        downloadLink.click();
    } else {
        console.log("No hay datos para descargar en el gráfico " + chartIndex);
    }
}

// Agregar listeners para cada botón de descarga
for (var i = 1; i <= 5; i++) {
    document.getElementById('downloadButton' + i).addEventListener('click', function (index) {
        return function () {
            downloadCSV(index);
        }
    }(i));
}



