/*********************************************************************************************************************
  ELABORADO POR CESAR CRIOLLO 21 AGOSTO 2024
  NUEVA VERSION 11 AGOSTO 2023
  ESTA VERSION FUNCIONA PERFECTA LAS OTRAS VERSIONES TIENEN UNA FALLA EN verifica_mac donde el k lo igualan a 4 se debe eliminar esa linea y todo fluye
  ESTA VERSION NO DEBE SER MODIFICADA POR MEDIDAS DE SEGURIDAD CUALQUIER CAMBIO DEBE REALIZARSE EN OTRO ARCHIVO
  EN ESTA VERSION LOGRAN COEXISTIR EL GRABADO DE DATOS EN SPIFFS Y LA LECTURA DE DATOS POR MODULO BLUETOOH

BIBLIOGRAFIA DE APOYO
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-websocket-server-arduino/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  PARA  LA INFORMACION DEL MODULO BLUETOOH TOMADO DE  https://wwww.prometec.net
  DATOS IMPORTANTES PARA LA MANIPULACION DE ARCHIVOS SPIFF EN https://github.com/me-no-dev/arduino-esp32fs-plugin 
  NOTAS;
  SE COLOCO esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE DEBIDO A QUE DESBORDABA EL TIEMPO DE BUSQUEDA CUANDO ERAN MUCHOS CLIENTES
********************************************************************************************************************/
// IMPORTANDO LAS LIBRERIAS NECESARIAS
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>      //libreria necesaria para verificar los clientes conectados en la lectura de las mac-address
#include <esp_task_wdt.h>  //libreria necesaria para manipular las funciones relacionadas con el wathdog del sistema

//LIBRERIA PARA LA MANIPULACIÓN MODULO BLUETOOH
#include "BluetoothSerial.h"  //libreria relacionada con el modulo bluetooh
BluetoothSerial SerialBT;     //se instancia la variable SerialBT

//LIBRERIAS PARA LA MANIPULACIÓN DE ARCHIVOS
#include "FS.h"      //libreria necesaria para manipulacion de archivos en el chip
#include "SPIFFS.h"  //combinado con la libreria anterior manipula formatos de archivos spiffs
/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true
#define MAXLIN 2080
#include "esp_task_wdt.h";  //ESTA LIBRERIA SE ENCARGA DE LA MANIPULACIÓN DEL WATCH DOG

File altura;
// DESCRIPCIÓN DE LAS CREDENCIALES DE LA RED WIFI Y SU RESPECTIVA CONTRASEÑA
const char *ssid = "DORAVILA2";      //SE ASIGNA EL NOMBRE A LA RED
const char *password = "987654321";  //SE ASIGNA LA CONTRASEÑA A LA RED
const int RELE = 4;                  //SE DEFINE EL RELE EN EL GPIO 4 LUEGO SE DECLARA  EN EL SETUP
bool ledState = 0;                   //ESTADO DEL LED

// ASIGNACION DE VARIABLES DEL SISTEMA
int permiso[100][6];  //CONTIENE INFORMACION DE LAS MAC ADDRESS PERMITIDAS
int acceso[20];
int AUXILIAR[5];  //ASOCIADO A LA FUNCION CARGADOR

wifi_sta_list_t stationList;  //PERMITE CONOCER LA CANTIDAD DE USUARIOS CONECTADOS AL SISTEMA

int NUN_CLIENT = 0;  //INDICA LA CANTIDAD DE CLIENTES CONECTADOS A LA RED

int mac1[100][7];  //se copia cada uno de los campos de la mac y al final la coincidencia

int acumula = 0;

int llave = 0;  //ES UNA BANDERA QUE PERMITE O NO EL ACCESO ES DECIR  LA ACTIVACIÓN DEL RELE
int APUNTADOR = 0;

int TABULADOR = 0;  //ESTA VARIABLE SE EMPLEA PARA LLEVAR CONTEO DE LOS DATOS QUE SE HAN INGRESADO POR EL MODULO BLUETOOHT
char Vrdata;        //ESTA VARIABLE SE EMPLEA PARA INGRESAR DATOS A LA MEMORIA INTERNA SSPIF

int hexa[3];     //ALMACENA INFORMACION DE LOS DATOS HEXADECIMALES QUE VAN INGRESANDO USADO EN LA FUNCION ANALIZADOR
int carril = 0;  //USADO EN LA FUNCION ANALIZADOR Y CONVERTIDOR
int decimal[7];  //ALMACENA LA CONVERSION DE HEXADECIMAL A DECIMAL EMPLEADO EN LA FUNCION ANALIZADOR Y CONVERTIDOR
int respuesta;

//char CUTRE[1000];
int CUENTA_LETRA = 0;
int CARRY = 0;  //LLEVA CONTEO DE SALTO DE LINEA  CODIGO 13-10-10 usado en la funcion INTERPRETAR

int LLEVO = 0;      //LLEVA LA CUENTA DE LETRAS LEIDAS EN LA FUNCION CARGADOR
int RESPUESTA = 0;  //LLEVA LA CONVERSION DE  LEIDAS EN LA FUNCION CARGADOR
int LINEA = 0;      //LLEVA LOS SALTOS DE LINEA EN EL ARCHIVO EMPLEADO EN LA FUNCION CARGADOR
int PUNTERO = 0;    //LLEVA LOS SALTOS DE LINEA EN EL ARCHIVO EMPLEADO EN LA FUNCION CARGADOR
int LAPIZ = 0;      //EMPLEADO EN LA FUNCION CARGADOR

int ONLY_ONE = 0;  //ES UNA BANDERA QUE PERMITE BORRAR LOS ARCHIVOS UNA SOLA VEZ HASTA QUE SE REINICIE EL SISTEMA

int TOTAL_ABONA2;  //ES UN CONTADOR QUE INDICA LA CANTIDAD DE CLIENTES QUE TIENE EL ARCHIVO QUE CONTIENE LA CANTIDAD DE CLIENTES AUTORIZADOS
int calculo = 0;   //ES UN CONTADOR QUE LLEVA LA CANTIDAD DE COINCIDNCIAS DE LAS MAC QUE INGRESAN  COMPARADA CON LA BASE DE DATOS

String datalog;  //LA DECLARACIÓN DE ESTE STRING PERMITE EL GRABADO DE DATOS EN LOS ARCHIVOS  SPIFS
String K;        //LA DECLARACIÓN DE ESTE STRING SE USA EN LA FUNCION readFile2

String SINCEL;  //LA DECLARACIÓN DE ESTE STRING SE USA EN LA FUNCION readFile2

//_________________________________________________________________________
String DIRECCION_IP;
//int LISTADO_IP[100][3];
int MEMORIA_CLIENTES = 0;  //EMPLEADO COMO CONTADOR PARA INDICAR LA CANTIDAD DE ABONADOS CONECTADOS  A LA RED
int LED;
int TABLA[100][10];  //ULTIMO OCTETO DE LA IP ,AUTORIZACION ,DIRECCION MAC
//__________________________________________________________________________
//__________________________________________________________________________
const int PIN_A = 18;
const int PIN_B = 19;
const int PIN_C = 16;
const int PIN_D = 17;
//__________________________________________________________________________
int PASOS;
int PROCESAR = 0;
int CUENTA_PASOS = 0;

int LIMITE = 400;  //EMPLEADO EN LA FUNCION HERON DEFINE CUANTO TIEMPO ACTIVARA EL MOTOR DE LA MAQUETA

int TEMPORAL[6];
int NUEVOMAC[6];  //COPIA TODAS LAS MAC ENTRANTES

int STATUS_PAGE = 0;  //DEFINE EN QUE PAGINA SE ESTA ACCEDIENDO
uint8_t caja = 0;


//VARIABLES EMPLEADAS EN LA FUNCION DE CONVERSION DE HEXADECIMAL A DECIMAL
int COCIENTE = 0;
int RESIDUO = 0;
int BANDERA = 1;
int RESTO = 0;
int contenedor[4];
int i;
//VARIABLES EMPLEADAS EN LA FUNCION RELOG
int MILISEGUNDOS = 0;
int SEGUNDOS = 0;
int MINUTOS = 0;
int HORAS = 0;
int DIAS = 0;
int MESES = 0;
int ANOS = 0;

int TIEMPO_ANTIGUO = 20;
int TIEMPO_NUEVO = 0;
//***********************************************
//ESTE GRUPO DE VARIABLES SON  EMPLEADAS EN LA FUNCION ALEATORIO
float copa1 = 0;
float copa2 = 0;
float copa3 = 0;
float copa4 = 0;
float copa5 = 0;
float copa6 = 0;

float copa7 = 0;
float copa8 = 0;
float copa9 = 0;
float copa10 = 0;
float copa11 = 0;
float copa12 = 0;

//***********************************************
char SUMERIO[20][20];
String SISTEMA = "";

typedef struct TABLA_SUMERIA {
  String SUMERIO_IP[30];
  int SUMERIO_MAC[30][6];
  int SUMERIO_ACCESO[30];
} TABLA_SUMERIA;


TABLA_SUMERIA GOBIERNO;

String gata;
float CILINDRO[20];

int PUNTO;

float TOTAL = 0;

float POTENCIAS[10] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
float ENTERO = 0;
float FRACCION = 0;
uint8_t LECTURAS = 0;
float dato1 = 0;
float dato2 = 0;
float RESPUESTA1 = 0;
float RESPUESTA2 = 0;
float DETERMINANTE = 0;
uint8_t TOKEN = 0;
float SIGNO = 1;
// SE CREA UN SERVIDOR WEB ASINCRONICO QUE SE COMUNICA POR EL PUERTO 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
//AsyncWebSocket ps("/ps");
//_________________________________________________________________________________________________
//
//_________________________________________________________________________________________________
//CONTENIDO DE LA PAGINA WEB INICIAL  MOSTRADA AL USUARIO
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP DORAVILA Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0af10a;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1 id="carta">SISTEMA DE ACCESO PORTON AUTOMATICO</h1>


<div class="card">

     
      <p class="state"> <span id="state1">  </span></p>
        
      <h1 class="state"> <span id="mac1"> </span>
        <!-- 
      : <span id="punta2">%STATE%</span>
      : <span id="punta3">%STATE%</span>
      : <span id="punta4">%STATE%</span>
      : <span id="punta5">%STATE%</span>
      : <span id="punta6">%STATE%</span>
      -->
    </div>
  </div>
  <div class="content">
    <div class="card">
    <p><button id="button" class="button" >CONFIRMAR</button></p>
    </div>

  
<h1>enviados:</h1>

  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
let puerta=1;
let macadd1=0;
let macadd2=0;
let macadd3=0;
let macadd4=0;
let macadd5=0;
let macadd6=0;

  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    let posicion=0;

if(puerta==1)
{
macadd1=event.data;  
document.getElementById('mac1').innerHTML = macadd1;
 // if(macadd1=="1")
 // {
 // document.getElementById('carta').innerHTML = "BIENVENIDO";
 // }
 
  //if(macadd1=="2")
 // {
 // document.getElementById('carta').innerHTML = "ACCESO DENEGADO";
 // }

}    

if(puerta==2)
{
macadd2=event.data;  
document.getElementById('state2').innerHTML = macadd2;

}

if(puerta==3)
{
  macadd3=event.data;
document.getElementById('punta3').innerHTML = macadd3;

}

if(puerta==4)
{
  macadd4=event.data;
document.getElementById('punta4').innerHTML = macadd4;

}

if(puerta==5)
{
  macadd5=event.data;
document.getElementById('punta5').innerHTML = macadd5;

}

if(puerta==6)
{
  macadd6=event.data;
document.getElementById('punta6').innerHTML = macadd6;

}

puerta=0;

document.getElementById('state1').innerHTML = event.data;
    if (event.data == "1")
    {
     puerta=1;
    }

    if (event.data == "2")
    {
      puerta=2;
    }

    if (event.data == "3")
    {
      puerta=3;
    }

    if (event.data == "4")
    {
      puerta=4;
    }

  if (event.data == "5")
    {
      puerta=5; 
    }

  if (event.data == "6")
    {
      puerta=6; 
    }

  if (event.data == "YES")
    {
      window.location.href = '/config';
    }

  if (event.data == "NO")
    {
      window.location.href = '/NEGADO';
    }

posicion=posicion+1;'32'
  }

  function onLoad(event) {
    initWebSocket();
    initButton();    
  }

  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }

window.addEventListener("load", onLoad,false);
</script>
</body>
</html>
)rawliteral";
//___________________________________________________________________________________________________________________________
//_____________________________________________________________________________________________________________________________
//CONTENIDO DE LA PAGINA WEB QUE MUESTRA AL PORTON ABRIENDOSE
const char paso_html[] PROGMEM = R"rawliteral(  
<!DOCTYPE html>
<title>pong</title>
<head>
    
</head>

<body>

<style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
   .state {
     font-size: 1.5rem;
     color:#8c8d8c;
     font-weight: bold;
   }

</style>
<script>
    let cu1=0 //este es el contador de posicion de la raqueta
    let cu2=0
	let xcentro=750//centro de masa x de la pelota   230
	let ycentro=110//centro de masa y de la pelota
	let x_raqueta=800//posicion inicial raqueta1
	let y_raqueta=80//posicion y de la raqueta1
	let x_raqueta2=200//posicion inicial raqueta1
	let y_raqueta2=80//posicion y de la raqueta1

	let hit=0 //esta variable indica si hay un choque de la pelota con alguna raqueta
	let time=0//esta variable lleva el tiempo de rrecorrido de la pelota 
	let vect=-1
  let con=0 //contador de apertura y cierre de la puerta;
	var milesec=0
	var seg=0
	var min=0
	var hor=0

/**********************************SUBRRUTINA PARA EL LIENZO*******************************************/
/******************************************************************************************/
    function escena()
        {
            var elemento=document.getElementById('lienzo'); 
            lienzo=elemento.getContext('2d'); 
            //lienzo.strokeRect(100,100,120,120); 
            var gradiente=lienzo.createLinearGradient(0,0,10,100); 
 			gradiente.addColorStop(0.5, '#0000FF'); 
 			gradiente.addColorStop(1, '#000000'); 
 			lienzo.fillStyle=gradiente;
            //lienzo.clearRect(120,120,80,80); 
            lienzo.clearRect(0,0,2500,1000);//SE LIMPIA EL LIENZO PARA EMPEZAR A DIBUJAR
            lienzo.beginPath();//SE PREPARA LA ZONA DE TRABAJO DEL LIENZO	
			ball();//se realiza dibujo de la pelota
      delay(3000);
			lienzo.font="bold 10px verdana, sans-serif";
			lienzo.font="bold 48px verdana, sans-serif";
			lienzo.stroke(); 
            lienzo.fill();

        }

		
		function ball()
		{
			lienzo.clearRect(0,0,2500,1000);
            lienzo.beginPath();
            lienzo.fillRect(x_raqueta,y_raqueta,10,100);//esta dibuja la raqueta movil
			lienzo.fillRect(x_raqueta2,y_raqueta2,10,100);//esta dibuja la raqueta movil del contrincante		
			lienzo.arc(xcentro,ycentro,20,0,Math.PI*2, false); //se dibuja la pelota
            lienzo.moveTo(xcentro+70,150); 

      if(con<=2)
        {

		        if(vect==-1)
					      {
						      xcentro=xcentro+1
					      }

			      if(vect==1)
					      {	
						      xcentro=xcentro-1
					      }
			      colision();//se verifica si no hay choque de la pelota con alguna raqueta
        }
	        
        milesec++
				if(milesec==1000)
				{
					seg++
					if(seg==60)
					{
						min++
							if(min==60)
							{
								hora++
							}
					}
				}
			lienzo.stroke(); 
            lienzo.fill();
			//window.setTimeout(escena,1000)	
		}
		function colision()
		{
			hit=0
			/***************************verifica choque raquetaA*********************/
			if((ycentro-y_raqueta)>=0&&(ycentro-y_raqueta)<=100)
			{
				if((x_raqueta-xcentro)>=568&&(x_raqueta-xcentro)<=569)
				{
					hit=1
					vect=-1
					con=con+1
				}	
			}	
			/***************************verifica choque raquetaB*********************/

			if((ycentro-y_raqueta2)>=0&&(ycentro-y_raqueta2)<=100)
			{
				if((xcentro-x_raqueta2)>=580&&(xcentro-x_raqueta2)<=581)
				{
					hit=1
					vect=1
          con=con+1
				}
			}	
		}	

/*********************************************************************************************/
		setInterval(escena,0)
/********************************************************************************/
window.addEventListener("load", escena, false);
/**************************************************************************************************/
</script>


 <div class="topnav">
    <h1>SISTEMA DE ACCESO PORTON AUTOMATICO</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>BIENVENIDO</h2>
      <!-- <p class="state"> <span id="state">%STATE%</span></p> -->
      
    </div>
                    <section id="cajalienzo"> 
                        <canvas id="lienzo" width="1000" height="400"> 
                        Su navegador no soporta el elemento canvas 
                        </canvas> 
                    </section> 
  </div>
    </body>
<html>

)rawliteral";
//FIN DEL CONTENIDO DE LA PAGINA WEB CONFIGURACION DE SISTEMAS
//___________________________________________________________________________________________________________________________
const char NEGADO[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  
  .topnav {
    overflow: hidden;
    background-color: #e20c0c;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #cfd307;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ACCESO DENEGADO CONSULTE AL ADMINISTRADOR DE SISTEMAS</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>STOP</h2>
      
    </div>
  </div>
</body>
</html>

)rawliteral";
//___________________________________________________________________________________________________________________________
const char CAPTURAMAC[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0af10a
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }


input:invalid {
border: 2px dashed red;
}
input:invalid:required {
background-image: linear-gradient(to right, pink, lightgreen);
}
input:valid {
border: 2px solid black;
}
  </style>
</head>
<body>
  <div class="topnav">
    <h1>SOLICITUD DE CONFIRMACION NUEVO INGRESO</h1>

<div class="card">     
      <h2>ESTADO: <span id="state1">%STATE%</span></h2>
      <h1 class="state">COORDENADAS
      : <span id="mac1">%STATE%</span>
      : <span id="punta2">%STATE%</span>
      : <span id="punta3">%STATE%</span>
      : <span id="punta4">%STATE%</span>
      : <span id="punta5">%STATE%</span>
      : <span id="punta6">%PORCINO%</span>
    </div>

<h1 id="password1">lectura:</h1>
<h1 id="password2">lectura:</h1>
<h1 id="password3">lectura:</h1>
<h1 id="password4">lectura:</h1>
<h1 id="password5">lectura:</h1>
<h1 id="password6">lectura:</h1>
<h2>macc</h2>

<h1 id="password7">lectura:</h1>
<h1 id="password8">lectura:</h1>
<h1 id="password9">lectura:</h1>
<h1 id="password10">lectura:</h1>
<h1 id="password11">lectura:</h1>
<h1 id="password12">lectura:</h1>

  </div>
  <div class="content">
    <div class="card"> 


      <h2>SOLICITE CODIGO DE ACCESO SEGUN COORDENADAS</h2>
      <br>
 <h2>PASSWORD A:<input type="text", name="PASSWORD"  required , id=PASSWORD1></h2> 
 <h2>PASSWORD B:<input type="text", name="PASSWORDB" required , id=PASSWORD2></h2>    
<input type="hidden" , name="mac1" value="20" id="LA1">
<input type="hidden" , name="mac2" value="20" id="LA2"> 
<input type="hidden" , name="mac3" value="20" id="LA3">   
<input type="hidden" , name="mac4" value="20" id="LA4">
<input type="hidden" , name="mac5" value="20" id="LA5"> 
<input type="hidden" , name="mac6" value="20" id="LA6">   

<input type="hidden" , name="mac7" value="20" id="LA7">
<input type="hidden" , name="mac8" value="20" id="LA8"> 
<input type="hidden" , name="mac9" value="20" id="LA9">   
<input type="hidden" , name="mac10" value="20" id="LA10">
<input type="hidden" , name="mac11" value="20" id="LA11"> 
<input type="hidden" , name="mac12" value="20" id="LA12">

      <h2><input type="submit" class= "button" id="button" , value="CONFIRMAR" onClick="button_on()" ></h2>
    </form>  
    </div>  
<h1>enviados:</h1>
  </div>
</div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
var puerta=0;
var macadd1=0;
var macadd2=0;
var LISTA=0;
var macadd3=0;
var macadd4=0;
var macadd5=0;
var macadd6=0;

var macadd7=0;
var macadd8=0;
var macadd9=0;
var macadd10=0;
var macadd11=0;
var macadd12=0;

let page=0;
let tranca=0;
let letras="desde mi esp32";


   nombre1=document.getElementById("PASSWORD1"); 
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
  
    document.getElementById('state1').innerHTML = event.data;
      if((puerta==1))
      { 
        document.getElementById('password1').innerHTML = event.data;
        document.getElementById('mac1').innerHTML = event.data;
        document.getElementById("LA1").value= event.data;
        macadd1=event.data;
      puerta=0;
      }   

      if((puerta==2))
      { 
        document.getElementById('password2').innerHTML = event.data;
        document.getElementById('punta2').innerHTML = event.data;
        document.getElementById("LA2").value= event.data;
      puerta=0;
      }  

      if((puerta==3))
      { 
        document.getElementById('password3').innerHTML = event.data;
        document.getElementById('punta3').innerHTML = event.data;
        document.getElementById("LA3").value= event.data;
      puerta=0;
      }  

      if((puerta==4))
      { 
        document.getElementById('password4').innerHTML = event.data;
        document.getElementById('punta4').innerHTML = event.data;
        document.getElementById("LA4").value= event.data;
      puerta=0;
      }  

      if((puerta==5))
      { 
        document.getElementById('password5').innerHTML = event.data;
        document.getElementById('punta5').innerHTML = event.data;
        document.getElementById("LA5").value= event.data;
      puerta=0;
      }  

      if((puerta==6))
      { 
        document.getElementById('password6').innerHTML = event.data;
        document.getElementById('punta6').innerHTML = event.data;
        document.getElementById("LA6").value= event.data;
      puerta=0;
      }  

      if((puerta==7))
      { 
        document.getElementById('password7').innerHTML = event.data;
        document.getElementById("LA7").value= event.data;
      puerta=0;
      }  

      if((puerta==8))
      { 
        document.getElementById('password8').innerHTML = event.data;
        document.getElementById("LA8").value= event.data;
      puerta=0;
      }  

      if((puerta==9))
      { 
        document.getElementById('password9').innerHTML = event.data;
        document.getElementById("LA9").value= event.data;
      puerta=0;
      }  

      if((puerta==10))
      { 
        document.getElementById('password10').innerHTML = event.data;
        document.getElementById("LA10").value= event.data;
      puerta=0;
      }  


      if((puerta==11))
      { 
        document.getElementById('password11').innerHTML = event.data;
        document.getElementById("LA11").value= event.data;
      puerta=0;
      }  


      if((puerta==12))
      { 
        document.getElementById('password12').innerHTML = event.data;
        document.getElementById("LA12").value= event.data;
      puerta=0;
      }  

    if (event.data == "ALE1")
    {
     puerta=1;
    }

    if (event.data == "ALE2")
    {
     puerta=2;
    }

    if (event.data == "ALE3")
    {
     puerta=3;
    }

    if (event.data == "ALE4")
    {
     puerta=4;
    }

    if (event.data == "ALE5")
    {
     puerta=5;
    }

    if (event.data == "ALE6")
    {
     puerta=6;
    }

    if (event.data == "MAC1")
    {
     puerta=7;
    }

   if (event.data == "MAC2")
    {
     puerta=8;
    }

   if (event.data == "MAC3")
    {
     puerta=9;
    }

  if (event.data == "MAC4")
    {
     puerta=10;
    }

 if (event.data == "MAC5")
    {
     puerta=11;
    }

 if (event.data == "MAC6")
    {
     puerta=12;
    }
  }

  function onLoad(event) 
  {
    initWebSocket();
    initButton();
  }

  function initButton()
   {
  document.getElementById('button').addEventListener('click', toggle);  
  }

  function toggle(){
    //websocket.send('Z');
    //websocket.send('A');
    //websocket.send('P');
    //websocket.send('E');
    letras=document.getElementById("PASSWORD1");
    websocket.send(letras.value);

    letras=document.getElementById("PASSWORD2");
    websocket.send(letras.value);
    INTERPRETE(nombre1.value)
    
  }

function button_on() 
{

window.location.href = '/REGISTRO';
}

function INTERPRETE(CARTA)
{
console.log("deberia mandar "+CARTA)

const cadenaNumero = CARTA.toString(); // Convertimos el número a cadena
//  const digitos = cadenaNumero.split('').map(Number); // Separamos los dígitos y los convertimos a números
const digitos = cadenaNumero.split(''); // Separamos los dígitos y los convertimos a números
const cantidadElementos = cadenaNumero.length;
console.log('Dígitos del número', CARTA, ':', digitos);
console.log(`El array tiene ${cantidadElementos} elementos.`);
}

window.addEventListener("load", onLoad,false);
</script>
</body>
</html>

)rawliteral";
//_____________________________________________________________________________________________________________________________
//_____________________________________________________________________________________________________________________________

char REGISTRAR[] PROGMEM = R"rawliteral(

<!DOCTYPE HTML>
<html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0af10a
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>SOLICITUD DE CONFIRMACION NUEVO INGRESO</h1>

<div class="card">
     
      <h2>ESTADO: <span id="state1">%STATE%</span></h2>
      <h1 class="state">COORDENADAS
      : <span id="mac1">%STATE%</span>
      : <span id="punta2">%STATE%</span>
      : <span id="punta3">%STATE%</span>
      : <span id="punta4">%STATE%</span>
      : <span id="punta5">%STATE%</span>
      : <span id="punta6">%STATE%</span>
    </div>


<h1 id="password1">lectura:</h1>
<h1 id="password2">lectura:</h1>
<h1 id="password3">lectura:</h1>
<h1 id="password4">lectura:</h1>
<h1 id="password5">lectura:</h1>
<h1 id="password6">lectura:</h1>
<h2>macc</h2>

<h1 id="password7">lectura:</h1>
<h1 id="password8">lectura:</h1>
<h1 id="password9">lectura:</h1>
<h1 id="password10">lectura:</h1>
<h1 id="password11">lectura:</h1>
<h1 id="password12">lectura:</h1>

  </div>
  <div class="content">
    <div class="card">
    <!--     <form action='http://192.168.4.2/PORTON/index.php' method="POST">--> 

    <form action='http://192.168.4.2/PORTON/index2.php' method="POST"> 
      <h2>FORMULARIO</h2>
      <br>
      <h2>CEDULA:<input type="int", name="CI" min="12" max="120" required ></h2>
      <h2>NOMBRES:<input type="text", name="NOMBRE" required ></h2>
      <h2>APELLIDOS:<input type="text", name="APELLIDO" required  ></h2>
      <h2>TELEFONO:<input type="text", name="TELEFONO" ></h2>
      <h2>ESTADO:<input type="text", name="ESTADO" required ></h2>
      <h2>MUNICIPIO:<input type="text", name="MUNICIPIO" required  ></h2>
      <h2>DIRECCION:<input type="text", name="DIRECCION" required ></h2>
      <h2>CORREO:<input type="text", name="CORREO" required></h2>
      <h2>PROFESION:<input type="text", name="PROFESION" required ></h2>
<input type="hidden" , name="mac1" value="20" id="LA1">
<input type="hidden" , name="mac2" value="20" id="LA2"> 
<input type="hidden" , name="mac3" value="20" id="LA3">   
<input type="hidden" , name="mac4" value="20" id="LA4">
<input type="hidden" , name="mac5" value="20" id="LA5"> 
<input type="hidden" , name="mac6" value="20" id="LA6">   

      <h2><input type="submit" id="button"  class= "button", value="CONFIRMAR" ></h2>
    </form>  
    </div>
<h1>enviados:</h1>
  </div>

<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
var puerta=1;
var macadd1=0;
var macadd2=0;
var LISTA=0;
var macadd3=0;
var macadd4=0;
var macadd5=0;
var macadd6=0;

var macadd7=0;
var macadd8=0;
var macadd9=0;
var macadd10=0;
var macadd11=0;
var macadd12=0;


let page=0;
let tranca=0;


//window.location.href = `http://192.168.4.1/NEGADO`

  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    let posicion=0;

if(tranca==0)
{

if(puerta==1)
{
macadd1=event.data;  
document.getElementById('password1').innerHTML = macadd1;
document.getElementById("LA1").value= macadd1;
}    

if(puerta==2)
{
LISTA=event.data;  
document.getElementById('password2').innerHTML = LISTA;
document.getElementById("LA2").value= LISTA;
}

if(puerta==3)
{
  macadd3=event.data;
document.getElementById('password3').innerHTML = macadd3;
document.getElementById("LA3").value= macadd3;
}

if(puerta==4)
{
  macadd4=event.data;
document.getElementById('password4').innerHTML = macadd4;
document.getElementById("LA4").value= macadd4;
}

if(puerta==5)
{
  macadd5=event.data;
document.getElementById('password5').innerHTML = macadd5;
document.getElementById("LA5").value= macadd5;
}

if(puerta==6)
{
  macadd6=event.data;
document.getElementById('password6').innerHTML = macadd6;
document.getElementById("LA6").value= macadd6;
}

if(puerta==7)
{
macadd7=event.data;
document.getElementById('password7').innerHTML = macadd7;
document.getElementById("LA7").value= macadd7;
}



if(puerta==8)
{
  macadd8=event.data;
document.getElementById('password8').innerHTML = macadd8;
document.getElementById("LA8").value= macadd8;
}


if(puerta==9)
{
  macadd9=event.data;
document.getElementById('password9').innerHTML = macadd9;
document.getElementById("LA9").value= macadd9;
}


if(puerta==10)
{
  macadd10=event.data;
  document.getElementById('password10').innerHTML = macadd10;

  document.getElementById('password10').innerHTML = macadd10;

document.getElementById("LA10").value= macadd10;
}


if(puerta==11)
{
  macadd11=event.data;
  document.getElementById('password11').innerHTML = macadd11;

document.getElementById("LA11").value= macadd11;
}


if(puerta==12)
{
  macadd12=event.data;
  document.getElementById('password12').innerHTML = macadd12;
document.getElementById("LA12").value= macadd12;
tranca=1;
}

}

puerta=0;

document.getElementById('state1').innerHTML = event.data;
    if (event.data == "1")
    {
     puerta=1;
    }

    if (event.data == "2")
    {
      puerta=2;
    }

    if (event.data == "3")
    {
      puerta=3;
    }

    if (event.data == "4")
    {
      puerta=4;
    }

  if (event.data == "5")
    {
      puerta=5; 
    }

  if (event.data == "6")
    {
      puerta=6; 
    }


  if (event.data == "7")
    {
      puerta=7; 
    }

  if (event.data == "8")
    {
      puerta=8; 
    }

  if (event.data == "9")
    {
      puerta=9; 
    }

  if (event.data == "10")
    {
      puerta=10; 
    }

  if (event.data == "11")
    {
      puerta=11; 
    }

  if (event.data == "12")
    {
      puerta=12; 
    }

  }

  function onLoad(event) 
  {
    initWebSocket();
    initButton();
    if(page==0)
    {
      page=1;
      websocket.send(3);
    }
  }

  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }
window.addEventListener("load", onLoad,false);
</script>
</body>
</html>
)rawliteral";

//____________________________________________________________________________________________________________________________________
//_________________________________________________________________________________________________
//CONTENIDO DE LA PAGINA WEB INICIAL  MOSTRADA AL USUARIO
const char URANIO[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP DORAVILA Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0af10a;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1 id="carta">SISTEMA DE ACCESO PORTON AUTOMATICO</h1>


<div class="card">

     
      <p class="state"> <span id="state1">  </span></p>
        
      <h1 class="state"> <span id="mac1"> </span>
        <!-- 
      : <span id="punta2">%STATE%</span>
      : <span id="punta3">%STATE%</span>
      : <span id="punta4">%STATE%</span>
      : <span id="punta5">%STATE%</span>
      : <span id="punta6">%STATE%</span>
      -->
    </div>
  </div>
  <div class="content">
    <div class="card">
    <p><button id="button" class="button" >CONFIRMAR</button></p>
    </div>

  
<h1>enviados:</h1>

  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
let puerta=1;
let macadd1=0;
let macadd2=0;
let macadd3=0;
let macadd4=0;
let macadd5=0;
let macadd6=0;

  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    let posicion=0;

if(puerta==1)
{
macadd1=event.data;  
document.getElementById('mac1').innerHTML = macadd1;
 // if(macadd1=="1")
 // {
 // document.getElementById('carta').innerHTML = "BIENVENIDO";
 // }
 
  //if(macadd1=="2")
 // {
 // document.getElementById('carta').innerHTML = "ACCESO DENEGADO";
 // }

}    

if(puerta==2)
{
macadd2=event.data;  
document.getElementById('state2').innerHTML = macadd2;

}

if(puerta==3)
{
  macadd3=event.data;
document.getElementById('punta3').innerHTML = macadd3;

}

if(puerta==4)
{
  macadd4=event.data;
document.getElementById('punta4').innerHTML = macadd4;

}

if(puerta==5)
{
  macadd5=event.data;
document.getElementById('punta5').innerHTML = macadd5;

}

if(puerta==6)
{
  macadd6=event.data;
document.getElementById('punta6').innerHTML = macadd6;

}

puerta=0;

document.getElementById('state1').innerHTML = event.data;
    if (event.data == "1")
    {
     puerta=1;
    }

    if (event.data == "2")
    {
      puerta=2;
    }

    if (event.data == "3")
    {
      puerta=3;
    }

    if (event.data == "4")
    {
      puerta=4;
    }

  if (event.data == "5")
    {
      puerta=5; 
    }

  if (event.data == "6")
    {
      puerta=6; 
    }

  if (event.data == "YES")
    {
      window.location.href = '/config';
    }

  if (event.data == "NO")
    {
      window.location.href = '/NEGADO';
    }

posicion=posicion+1;'32'
  }

  function onLoad(event) {
    initWebSocket();
    initButton();    
  }

  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }

window.addEventListener("load", onLoad,false);
</script>
</body>
</html>
)rawliteral";
//___________________________________________________________________________________________________________________________


//__________________________________________________________________________________________________________________________________
//INICIO DE FUNCIONES RELACIONADAS CON LA MANIPULACION DE ARCHIVOS FORMATO SPIFFS

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  // Serial.printf("MOSTRANDO DIRECTORIO: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- FALLA AL ABRIR EL DIRECTORIO");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - NO HAY DIRECTORIOS");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  ARCHIVO: ");
      Serial.print(file.name());
      Serial.print("\tTAMAÑO: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("LEYENDO ARCHIVOS: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- FALLA EN LA LECTURA DE ARCHIVOS");
    return;
  }

  Serial.println("- LECTURA DE ARCHIVO:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

/***********************************************************************/
void readFile2(fs::FS &fs, const char *path) {
  //Serial.printf("LEYENDO ARCHIVOS: %s\r\n", path);
  CUENTA_LETRA = 0;
  datalog = String(K);
  File file = fs.open(path);


  if (!file || file.isDirectory()) {
    Serial.println("- FALLA EN LA LECTURA DE ARCHIVOS");
    return;
  }

  //Serial.println("- LECTURA DE ARCHIVO:");
  while (file.available()) {
    //Serial.write(file.read());
    K = file.read();
    //Serial.printf("ahora si %s \n",K);
    CUENTA_LETRA++;
    Serial.print(K);
    INTERPRETAR();
  }
  file.close();
  Serial.printf("cuenta letra dio:%d \n ", CUENTA_LETRA);
}

/**********************************************************************/
void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("GRABANDO ARCHIVO: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- FALLA AL ABRIR EL ARCHIVO PARA ESCRITURA");
    return;
  }
  if (file.print(message)) {
    Serial.println("- ARCHIVO GRABADO");
  } else {
    Serial.println("- FALLA AL GRABAR EL ARCHIVO");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- FALLA AL ABRI EL ARCHIVO");
    return;
  }
  if (file.print(message)) {
    Serial.println("- DATOS AGREGADOS");
  } else {
    Serial.println("- FALLA AL AGREGAR DATOS");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("- ARCHIVO RENOMBRADO");
  } else {
    Serial.println("- FALLA AL CAMBIAR EL NOMBRE DEL ARCHIVO");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("- ARCHIVO BORRADO");
  } else {
    Serial.println("- FALLA AL BORRAR ARCHIVOS ");
  }
}

void testFileIO(fs::FS &fs, const char *path) {
  Serial.printf("Testing file I/O with %s\r\n", path);

  static uint8_t buf[512];
  size_t len = 0;
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- FALLA AL ABRIR ARCHIVO PARA ESCRITURA");
    return;
  }

  size_t i;
  Serial.print("- writing");
  uint32_t start = millis();
  for (i = 0; i < 2048; i++) {
    if ((i & 0x001F) == 0x001F) {
      Serial.print(".");
    }
    file.write(buf, 512);
  }
  Serial.println("");
  uint32_t end = millis() - start;
  Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
  file.close();

  file = fs.open(path);
  start = millis();
  end = start;
  i = 0;
  if (file && !file.isDirectory()) {
    len = file.size();
    size_t flen = len;
    start = millis();
    Serial.print("- LEYENDO");
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      if ((i++ & 0x001F) == 0x001F) {
        Serial.print(".");
      }
      len -= toRead;
    }
    Serial.println("");
    end = millis() - start;
    Serial.printf("- %u BYTES LEIDOS EN %u ms\r\n", flen, end);
    file.close();
  } else {
    Serial.println("- FALLA AL LEER ARCHIVO ");
  }
}
//FINAL DE FUNCIONES RELACIONADAS CON MANIPULACION DE ARCHIVOS FORMATO SPIFFS
//___________________________________________________________________________
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {

  //Serial.println("PULSADO EL BOTON");
  caja = *data;
  //Serial.println("MENSAJE ENVIADO DESDE LA PAGINA");
  Serial.println(caja);
  //caja=53;
}
//___________________________________________________________________________________________________________________________
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) 

{
  Serial.println("------------------------------------------NUEVA ORDEN------------------------------------------------------");
 
  DIRECCION_IP = (client->remoteIP().toString().c_str());
  Serial.printf("LECTURA DE  DIRECCION IP  %s\n", DIRECCION_IP);
  identidad();

  switch (type) {

    case WS_EVT_CONNECT:
      //Serial.printf("CLIENTE WEBSOCKET  #%u CONECTADO %s\n", client->id(), client->remoteIP().toString().c_str());
      Serial.printf("INTENTANDO ACCEDER A LA  PAGINA %d \n",STATUS_PAGE);
      if(STATUS_PAGE==4)
      {
       RANDOM();
      }


      DIRECCION_IP = (client->remoteIP().toString().c_str());
      //
      //Serial.println("LISTA LA VERIFICACION DE USUARIOS");
      Serial.println("---------------------------------------------------------------------------------");
      /*
  Serial.printf("CLIENTE WEBSOCKET  #%u CONECTADO %s\n", client->id(), client->remoteIP().toString().c_str());
 Serial.printf("TRADUCIDO  %d\n", DIRECCION_IP);
Serial.print(DIRECCION_IP[0]);
Serial.print(DIRECCION_IP[1]);
Serial.print(DIRECCION_IP[2]);
Serial.print(DIRECCION_IP[3]);
Serial.print(DIRECCION_IP[4]);
Serial.print(DIRECCION_IP[5]);
Serial.print(DIRECCION_IP[6]);
Serial.print(DIRECCION_IP[7]);
Serial.print(DIRECCION_IP[8]);
Serial.print(DIRECCION_IP[9]);
Serial.print(DIRECCION_IP[10]);
*/

      esp_wifi_ap_get_sta_list(&stationList);
      NUN_CLIENT = stationList.num;
      Serial.printf("CANTIDAD DE CLIENTES CONECTADOS : %d \n", NUN_CLIENT);
      //Serial.printf("LISTADO DE MACCADDRRESS CONECTADAS AL SISTEMA: \n");
      if (NUN_CLIENT > MEMORIA_CLIENTES)  //SI EL NUMERO DE CLIENTES CONECTADOS ES MAYOR AL ANTERIOR ES PORQUE HAY UNA NUEVA CONEXION
      {
        MEMORIA_CLIENTES = NUN_CLIENT;
        //LISTADO_IP[NUN_CLIENT]=DIRECCION_IP;//SE VAN GRABANDO EL LISTADO DE LAS IP QUE INGRESAN
        Serial.println("---------------------------------------------------------------------------------");
        Serial.println("ALGUIEN SE HA CONECTADO DEL SISTEMA :");
        Serial.printf("CON DIRECCION IP  %s\n", DIRECCION_IP);

        GOBIERNO.SUMERIO_IP[NUN_CLIENT - 1] = DIRECCION_IP;

        Serial.printf("CON MACC ADDRESS  ");
        wifi_sta_info_t station = stationList.sta[NUN_CLIENT - 1];
        for (int i = 0; i < 6; i++) {
          Serial.printf(" mac %d  ", station.mac[i]);

          GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][i] = station.mac[i];
          if (i < 5) {
            Serial.print(":");
          }
          //esta parte verifica la macc con la matriz permiso
        }
        Serial.printf("\n");
      }


      Serial.printf("\n en el archivo a leer existen %d registros ", LINEA);
      for (int j = 0; j < LINEA; j++) {
        calculo = 0;
        for (int l = 0; l < 6; l++) {
          Serial.printf("\n LEYENDO DATO %d  COMPARANDO CON: %d", permiso[j][l], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][l]);
          if (permiso[j][l] == GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][l]) {
            calculo++;
          }
        }
        //
        if (calculo == 6) {
          GOBIERNO.SUMERIO_ACCESO[NUN_CLIENT - 1] = 1;
          Serial.printf("\nCOINCIDENCIA TOTAL EN ESTE PUNTO");
          Serial.printf("\n EST0 ES LO QUE SE HA GRABADO HASTA EL MOMENTO EN ESE REGISTRO:");
          Serial.printf("ip %s  con direccion macc    %d:%d:%d:%d:%d:%d   ACCESO TIPO :%d \n", GOBIERNO.SUMERIO_IP[NUN_CLIENT - 1], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][0], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][1], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][2], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][3], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][4], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][5], GOBIERNO.SUMERIO_ACCESO[NUN_CLIENT - 1]);
          j=LINEA; //COMO YA ENCONTRO COINCIDENCIA TOTAL SE DEBE SALIR DEL CICLO FOR        
        }

        if (calculo != 6) {
          GOBIERNO.SUMERIO_ACCESO[NUN_CLIENT - 1] = 0;
          Serial.printf("\nNO EXISTE COINCIDENCIA EN ESTE PUNTO");
          Serial.printf("\n EST0 ES LO QUE SE HA GRABADO HASTA EL MOMENTO EN ESE REGISTRO:");
          Serial.printf("ip %s  con direccion macc    %d:%d:%d:%d:%d:%d   ACCESO TIPO :%d \n", GOBIERNO.SUMERIO_IP[NUN_CLIENT - 1], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][0], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][1], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][2], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][3], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][4], GOBIERNO.SUMERIO_MAC[NUN_CLIENT - 1][5], GOBIERNO.SUMERIO_ACCESO[NUN_CLIENT - 1]);
        }
        
      }
      // verifica_cliente();
      break;
    case WS_EVT_DISCONNECT:
      Serial.println("---------------------------------PAGINA------------------------------------------------");

      esp_wifi_ap_get_sta_list(&stationList);
      NUN_CLIENT = stationList.num;
      Serial.printf("CANTIDAD DE CLIENTES CONECTADOS : %d \n", NUN_CLIENT);
      //Serial.printf("LISTADO DE MACCADDRRESS CONECTADAS AL SISTEMA: \n");

      if (NUN_CLIENT < MEMORIA_CLIENTES)  //SI EL NUMERO DE CLIENTES CONECTADOS ES MAYOR AL ANTERIOR ES PORQUE HAY UNA NUEVA CONEXION
      {
        MEMORIA_CLIENTES = NUN_CLIENT;
        //LISTADO_IP[NUN_CLIENT]=DIRECCION_IP;//SE VAN GRABANDO EL LISTADO DE LAS IP QUE INGRESAN
        Serial.println("ALGUIEN SE HA DESCONECTADO DEL SISTEMA :");
      }
      //cliente_fuera();
      break;
    case WS_EVT_DATA:

      //DIRECCION_IP=(client->remoteIP().toString().c_str());
      //Serial.println("---------------------------------------------------------------------------------");
      //Serial.printf("ALGUIEN A PULSADO UN BOTON FUE %s\n",  client->remoteIP().toString().c_str());
      //Serial.printf("LISTADO DE INFORMACION\n");
      for (int i = 0; i < NUN_CLIENT; i++) {
        //Serial.printf("ip %s  con direccion macc    %d:%d:%d:%d:%d:%d   ACCESO TIPO :%d \n",GOBIERNO.SUMERIO_IP[i],GOBIERNO.SUMERIO_MAC[i][0],GOBIERNO.SUMERIO_MAC[i][1],GOBIERNO.SUMERIO_MAC[i][2],GOBIERNO.SUMERIO_MAC[i][3],GOBIERNO.SUMERIO_MAC[i][4],GOBIERNO.SUMERIO_MAC[i][5],GOBIERNO.SUMERIO_ACCESO[i]);

        if (GOBIERNO.SUMERIO_MAC[i][0] == 0)  //ESTO SUCEDE CUANDO HAY ERROR DE LECTURA SE PROCEDE A RESETEAR AUTOMATICAMENTE EL CHIP ES UN PROBLEMA INTERNO DEL DISPOSITIVO
        {
          ESP.restart();  //SE REALIZA UN RESETEO AUTOMATICO PARA CARGAR EL SISTEMA CON LOS NUEVOS DATOS
        }

        if (GOBIERNO.SUMERIO_IP[i] == client->remoteIP().toString().c_str()) {
          RESTO = i;
        }
      }

      if (STATUS_PAGE == 4) {

        CAPTURAR(len, data);
        Serial.printf("\n los valores obtenidos de la captura fueron ;%f y %f", dato1, dato2);
        COMPROBAR();
      }


      if (STATUS_PAGE == 5) {
        Serial.println("AQUI DEBERIA GRABAR LOS DATOS EN EL CHIP ");

        STATUS_PAGE = 56;
      }

      if (GOBIERNO.SUMERIO_ACCESO[RESTO] == 1) {
        Serial.printf("ACCESO CONCEDIDO\n");
        //STATUS_PAGE=1;
            if(STATUS_PAGE==1)
            {
             PROCESAR = 1; 
             ABRE_PORTON();
            } 
        
        ws.textAll(String(1));
        delay(35);
        ws.textAll(String("YES"));
        delay(35);
      } 
      
      if (GOBIERNO.SUMERIO_ACCESO[RESTO] != 1) {
        ws.textAll(String(1));
        delay(35);
        ws.textAll(String("NO"));
        delay(35);
       // TOKEN = 0;
      }
      handleWebSocketMessage(arg, data, len);
      //Serial.printf("CLIENTE WEBSOCKET  #%u mensaje %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_PONG:
      Serial.println("--------------hola----------------------------------------------------------");

    case WS_EVT_ERROR:
    Serial.println("--------------ERROR EN EL SISTEMA----------------------------------------------------------");
      break;
  }
   NMAP();
}
//_________________________________________________________________________________________________________________________

void CAPTURAR(size_t len, uint8_t *data) {

  Serial.println("ANALISIS DE CAPTURA DE DATOS");
  Serial.printf("\nEL MENSAJE FUE %s", data);
  Serial.printf(" \n TAMAÑO DEL MENSAJE ES %d \n", len);

  PUNTO = len + 1;
  SIGNO = 1;
  for (int i = 1; i <= len; i++) {
    gata = *(data - 1 + i);
    Serial.printf("\n PARTE DEL DATO ES: %s", gata);

    if (gata == "46")  //IDENTIFICA EL PUNTO DECIMAL
    {
      CILINDRO[i] = 100;
      PUNTO = i;
    }

    if (gata == "44")  //IDENTIFICA SI HAY UNA COMA
    {
      CILINDRO[i] = 100;
      PUNTO = i;
    }


    if (gata == "45")  //IDENTIFICA SI HAY UN NUMERO NEGATIVO
    {
      CILINDRO[i] = -1;
      SIGNO = -1;
    }


    if (gata == "48") {
      CILINDRO[i] = 0;
    }

    if (gata == "49") {
      CILINDRO[i] = 1;
    }

    if (gata == "50") {
      CILINDRO[i] = 2;
    }

    if (gata == "51") {
      CILINDRO[i] = 3;
    }

    if (gata == "52") {
      CILINDRO[i] = 4;
    }

    if (gata == "53") {
      CILINDRO[i] = 5;
    }

    if (gata == "54") {
      CILINDRO[i] = 6;
    }

    if (gata == "55") {
      CILINDRO[i] = 7;
    }

    if (gata == "56") {
      CILINDRO[i] = 8;
    }

    if (gata == "57") {
      CILINDRO[i] = 9;
    }
  }

  Serial.println("DATOS PROCESADOS DE LA CAPTURA: ");
  for (int i = 1; i <= len; i++) {

    Serial.printf("\n %f", CILINDRO[i]);
  }

  ENTERO = 0;

  Serial.println(" PARTE ENTERA DEL SISTEMA: ");
  for (int j = 1; j < PUNTO; j++) {

    if (CILINDRO[PUNTO - j] != -1) {
      Serial.printf("\n %f", CILINDRO[PUNTO - j]);
      ENTERO = ENTERO + POTENCIAS[j - 1] * CILINDRO[PUNTO - j];
    }
  }


  Serial.printf("\n AL FINAL EL ENTERO DIO: %f", ENTERO);


  Serial.printf("\n PARTE DECIMAL DEL SISTEMA:\n ");
  FRACCION = ENTERO;

  for (int i = PUNTO + 1; i <= len; i++) {

    Serial.printf("\n se va a dividir %f con %f= %f", CILINDRO[i], POTENCIAS[i - PUNTO], CILINDRO[i] / POTENCIAS[i - PUNTO]);
    FRACCION = FRACCION + (CILINDRO[i] / POTENCIAS[i - PUNTO]);
    Serial.printf("\n la fraccion lleva %f", FRACCION);
  }


  FRACCION = FRACCION * SIGNO;
  LECTURAS = LECTURAS + 1;

  Serial.printf("\n AL FINAL EL DECIMAL DIO: %f LLEVA %d LECTURAS", FRACCION, LECTURAS);


  Serial.printf("\n NUEVA ORDEN");


  if (LECTURAS == 1) {
    dato1 = FRACCION;
  }

  if (LECTURAS == 2) {
    dato2 = FRACCION;
    LECTURAS = 0;
  }
}

//_________________________________________________________________________________________________________________________
void identidad()  //ESTA FUNCION VERIFICA QUIENES SE HAN CONECTADO A LA RED
{
  // Serial.println("viendo quien esta aqui);
  //Serial.printf("BUSCANDO QUIEN ESTA AQUI\n");
  for (int i = 0; i < NUN_CLIENT; i++) {
    //Serial.printf("ip %s  con direccion macc    %d:%d:%d:%d:%d:%d   ACCESO TIPO :%d \n",GOBIERNO.SUMERIO_IP[i],GOBIERNO.SUMERIO_MAC[i][0],GOBIERNO.SUMERIO_MAC[i][1],GOBIERNO.SUMERIO_MAC[i][2],GOBIERNO.SUMERIO_MAC[i][3],GOBIERNO.SUMERIO_MAC[i][4],GOBIERNO.SUMERIO_MAC[i][5],GOBIERNO.SUMERIO_ACCESO[i]);
    if (GOBIERNO.SUMERIO_MAC[i][0] == 0)  //ESTO SUCEDE CUANDO HAY ERROR DE LECTURA SE PROCEDE A RESETEAR AUTOMATICAMENTE EL CHIP ES UN PROBLEMA INTERNO DEL DISPOSITIVO
    {
      ESP.restart();  //SE REALIZA UN RESETEO AUTOMATICO PARA CARGAR EL SISTEMA CON LOS NUEVOS DATOS
    }

    if (GOBIERNO.SUMERIO_IP[i] == DIRECCION_IP) {
      RESTO = i;
    }
  }
}

//_________________________________________________________________________________________________________________________
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
//________________________________________________________________________________________________________________________

String PINCEL(const String &var) {

  Serial.println("AHORA VISUALIZANDO LO QUE HA ENVIADO EL PINCEL");
  Serial.printf("%s", var);
  Serial.println(" ");
  return String();
}
//_________________________________________________________________________________________________________________________
String processor(const String &var) {
  Serial.println("VISUALIZANDO LO QUE SUCEDE EN EL AMBIENTE ");
  Serial.println(var);
  if (var == "STATE") {
    if (ledState) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  return String();
}
//________________________________________________________________________________________________________________________
void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);     //Se define la velocidad o formato en que seran escritos los datos en el monitor serial
  pinMode(RELE, OUTPUT);    //se define el pin de nombre rele como salida digital
  digitalWrite(RELE, LOW);  //SE arranca el rele apagado
  pinMode(PIN_A, OUTPUT);   //se define el pin A como salida digital
  pinMode(PIN_B, OUTPUT);   //se define el pin B como salida digital

  //SE ARRANCA VERIFICANDO EL MANEJO DE ARCHIVOS  EN EL FORMATO SPIFFS
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS FALLA AL MONTAR LOS ARCHIVOS REVISE SISTEMA");
  }

  //writeFile(SPIFFS, "/hello.txt", "Hola ");
  listDir(SPIFFS, "/", 0);
  Serial.println("MOSTRANDO EL CONTENIDO DEL ARCHIVO HELLO.TXT");
  readFile(SPIFFS, "/hello.txt");
  //delay(5000);
  //esp_task_wdt_reset_user() ;
  CARGADOR(SPIFFS, "/hello.txt");  //SE GRABAN LOS DATOS DEL ARCHIVO INTERNO AL ARRAY PERMISO EL CUAL CONTIENE LOS CLIENTES AUTORIZADOS
  TOTAL_ABONA2 = LINEA;

  readFile2(SPIFFS, "/hello.txt");
  //writeFile(SPIFFS, "/hello.txt", "abc ");
  //appendFile(SPIFFS, "/hello.txt", "Mundo Cruel\r\n");
  //readFile2(SPIFFS, "/hello.txt");
  /*   
    listDir(SPIFFS, "/", 0);
    writeFile(SPIFFS, "/hello.txt", "Hola ");
    //listDir(SPIFFS, "/", 0);
    //appendFile(SPIFFS, "/hello.txt", "Hola");
    appendFile(SPIFFS, "/hello.txt", "Mundo Cruel\r\n");
    appendFile(SPIFFS, "/hello.txt", "COMO TE VA EL DIA DE HOY\r\n");
    readFile(SPIFFS, "/hello.txt");
    //renameFile(SPIFFS, "/hello.txt", "/foo.txt");
    //readFile(SPIFFS, "/foo.txt");
    //deleteFile(SPIFFS, "/foo.txt");
    //testFileIO(SPIFFS, "/test.txt");
    //deleteFile(SPIFFS, "/test.txt");
    Serial.println( "PRUEBA COMPLETADA" );
*/
  //MAC ADDRESS DEL PRIMER CLIENTE AUTORIZADO CESAR
  permiso[0][0] = 152;
  permiso[0][1] = 246;
  permiso[0][2] = 33;
  permiso[0][3] = 164;
  permiso[0][4] = 223;
  permiso[0][5] = 75;
  /***********************************************************************************/
  Serial.println("_______________________________________________________________________________________________");
  Serial.println("_____________________________TRADUCIDO DE HEXADECIMAL  A DECIMAL ______________________________");

  //mostrando lo que contiene la matriz permisos
  GRABAR_MATRIZ();
  Serial.printf(" SE HAN LEIDO UNA CANTIDAD DE %d CLIENTES EN EL ARCHIVO hello.txt \n ", TOTAL_ABONA2);
  /***********************************************************************************************/
  SerialBT.begin("DORAVIA_ESP32");  //NOMBRE ASIGNADO AL MODULO BLUETOOTH DEL ESP 32
  /**********************************************************************************************/
  //pinMode(ledPin, OUTPUT);
  //digitalWrite(ledPin, LOW);
  // Connect to Wi-Fi
  /*
  WiFi.begin(ssid, password);//EN MODO ESTACION

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);   
    Serial.println("BUSCANDO CONECTAR A WIFI...");
  }
*/

  IPAddress local_ip(192, 168, 4, 1);  //SE ASIGNA LA DIRECCION IP FIJA AL SERVIDOR
  IPAddress gateway(192, 168, 4, 1);   //SE ASIGNA EL GATEWAY DEL SISTEMA EL CHIP ES EL GATEWAY
  IPAddress subnet(255, 255, 255, 0);  //SE ASIGNA LA MASCARA DE SUBNET
  WiFi.softAP(ssid, password);         //SE CONFIGURA LA RED EN MODO DE TRABAJO TIPO  ACCES POINT
  //WiFi.softAP(ssid, password,1,1,4); AVERIGUAR COMO SE DEJA OCULTA LA RED
  WiFi.softAPConfig(local_ip, gateway, subnet);

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  initWebSocket();  //ARRANCA EL WEBSOCKET
  // COMANDOS DONDE ARRANCA LA PAGINA WEB QUE MOSTRARA EL SERVIDOR
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
    STATUS_PAGE = 1;
    //TOKEN = 0;
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", paso_html, processor);
    //STATUS_PAGE=2; //DEJARLO DESACTIVADO SI SE COLOCA CAUSA PROBLEMAS LA PUERTA SOLO CIERRA
    //TOKEN = 0;
  });

  server.on("/NEGADO", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", NEGADO, processor);
    STATUS_PAGE = 3;
    //TOKEN = 0;
  });

  server.on("/CAPTURA", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", CAPTURAMAC, processor);
   STATUS_PAGE = 4;
    //TOKEN = 0;
  });


  server.on("/REGISTRO", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.printf("INTENTANDO ACCEDER AL LAS OPCIONES DE REGISTRO LA DIRECCION : %s",GOBIERNO.SUMERIO_IP[RESTO]);

    if (TOKEN == 2) {
      request->send_P(200, "text/html", REGISTRAR, processor);
    }

    else {
      request->send_P(200, "text/html", NEGADO, processor);
      
    }

    STATUS_PAGE = 5;
  });



server.on("/URANIO", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", URANIO, processor);
   STATUS_PAGE = 6;
    //TOKEN = 0;
  });




  server.begin();  //INICIA EL SERVIDOR
  //ESP.wdtDisable();
  //PARA QUE LAS VARIABLES ASOCIADAS A CAPTURA NO INICIALIZEN EN CERO
RANDOM();
}
//___________________________________________________________________________________________________________________
void loop() {
  RELOG();
  ws.cleanupClients();
  esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
  //ESTE GRUPO DE LINEAS VA ASOCIADO A LA ENTRADA DE DATOS DEL MODULO BLUE TOOTH
  if (Serial.available())  // SI LLEGAN DATOS POR EL BLUE TOOTH REALIZA EL BUCLE
    SerialBT.write(Serial.read());
  if (SerialBT.available())  // Lo que llega del bluetooth
  {
    Serial.printf("only one vale =%d \n", ONLY_ONE);
          while (ONLY_ONE == 0)
          {
            writeFile(SPIFFS, "/hello.txt", "");  //SE BORRA CUALQUIER CONTENIDO DEL ARCHIVO ANTERIOR Y SE REESCRIBE SOBRE EL
            ONLY_ONE = 1;
            }
    String S = GetLine();  // GetFile(SPIFFS, "/hello.txt");//
    datalog = String(S);
    Serial.print(datalog);
    //_________________________________________________________________
    //_________________________________________________________________
    appendFile(SPIFFS, "/hello.txt", datalog.c_str());  //SE ESCRIBE EN  EL ARCHIVO HELLO.TXT INTERNO DEL ESP32
    readFile(SPIFFS, "/hello.txt");                     //SE MUESTRAN LOS DATOS ACTUALIZADOS
    ONLY_ONE = 1;                                       //SE COLOCA LA BANDERA ONLY_ONE EN UNO
                                                        //datalog = readFile(SPIFFS, "/hello.txt");

    //esto es de prueba
    Serial.println("________________________________________________________________");
    Serial.println("__________MOSTRANDO EL ARCHIVO ACTUALIZADO______________________");
    //readFile2(SPIFFS, "/hello.txt");
    //GetFile(SPIFFS, "/hello.txt");
    //for(int i=0; i<=10;i++)
    //{
    //  Serial.print(CUTRE[i]);
    //}
    //ESCULPIR();
    //________________________
    GRABAR_MATRIZ();  //SE GRABAN LOS DATOS EN LA MATRIZ
    ESP.restart();    //SE REALIZA UN RESETEO AUTOMATICO PARA CARGAR EL SISTEMA CON LOS NUEVOS DATOS
  }
  //ONLY_ONE =0; no retornarlo a cero por que solo grabara la ultima linea

  if (STATUS_PAGE == 1) {
    HERON();  //ESTE LLAMADO ES A LA FUNCION QUE SE ENCARGA DE MOVER EL MOTOR APERTURA DEL PORTON
  }

  if (STATUS_PAGE == 4) {
    //Serial.println("_____________MOSTRANDO LA MAC A ENVIAR____________________");
    //Serial.printf("DIRECCION MAC_ADDRESS: %d:%d:%d:%d:%d:%d \n",NUEVOMAC[0],NUEVOMAC[1],NUEVOMAC[2],NUEVOMAC[3],NUEVOMAC[4],NUEVOMAC[5]);

    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE

    ALEATORIO();           //SE ENVIA UN GRUPO DE NUMEROS ALEATORIOS PARA LUEGO SER RESUELTOS
    SEND_MAC();            //SE ENVIA LA MAC_ADDRESS DEL SISTEMA DEL ULTIMO EQUIPO CONECTADO AL SISTEMA
    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    CALCULO();
  }

  if (STATUS_PAGE == 5) {
    //Serial.println("_____________MOSTRANDO LA MAC A ENVIAR____________________");
    //Serial.printf("DIRECCION MAC_ADDRESS: %d:%d:%d:%d:%d:%d \n",GOBIERNO.SUMERIO_MAC[RESTO][0],GOBIERNO.SUMERIO_MAC[RESTO][1],GOBIERNO.SUMERIO_MAC[RESTO][2],GOBIERNO.SUMERIO_MAC[RESTO][3],GOBIERNO.SUMERIO_MAC[RESTO][4],GOBIERNO.SUMERIO_MAC[RESTO][5]);

    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    //SEND_MAC();
    /*************************************************/
    //esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    ws.textAll(String(1));
    delay(180);
    ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][0]));
    delay(180);

    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    ws.textAll(String(2));
    delay(180);
    ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][1]));
    delay(180);

    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    ws.textAll(String(3));
    delay(180);
    ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][2]));
    delay(180);

    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    ws.textAll(String(4));
    delay(180);
    ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][3]));
    delay(180);

    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    ws.textAll(String(5));
    delay(180);
    ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][4]));
    delay(180);

    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    ws.textAll(String(6));
    delay(180);
    ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][5]));
    delay(180);
    /*********************ESTE TROZO DE CODIGO ES TEMPORAL PARA PROBAR GRABADO DE SISTEMA****************************/
    /*
          Serial.println("proceso de grabado en el chip ");
          Serial.printf("\n grabar %d:%d:%d:%d:%d:%d:",NUEVOMAC[0],NUEVOMAC[1],NUEVOMAC[2],NUEVOMAC[3],NUEVOMAC[4],NUEVOMAC[5]);
//GRABAR_CHIP();
/************************************************/
    //  delay(6000);
    //Serial.println("_____________PREPARADO PARA GRABAR ESTOS DATOS ____________________");
    if (caja == 53) {
      NUEVOMAC[0] = GOBIERNO.SUMERIO_MAC[RESTO][0];
      NUEVOMAC[1] = GOBIERNO.SUMERIO_MAC[RESTO][1];
      NUEVOMAC[2] = GOBIERNO.SUMERIO_MAC[RESTO][2];
      NUEVOMAC[3] = GOBIERNO.SUMERIO_MAC[RESTO][3];
      NUEVOMAC[4] = GOBIERNO.SUMERIO_MAC[RESTO][4];
      NUEVOMAC[5] = GOBIERNO.SUMERIO_MAC[RESTO][5];

      Serial.println("proceso de grabado en el chip ");
      Serial.printf("\n grabar %d:%d:%d:%d:%d:%d:", NUEVOMAC[0], NUEVOMAC[1], NUEVOMAC[2], NUEVOMAC[3], NUEVOMAC[4], NUEVOMAC[5]);
      caja = 0;
      GRABAR_CHIP();
      String S = SISTEMA;  // GetFile(SPIFFS, "/hello.txt");//
      datalog = String(S);
      Serial.print(datalog);
      //_________________________________________________________________
      //_________________________________________________________________
      appendFile(SPIFFS, "/hello.txt", datalog.c_str());

      Serial.println("_____________PREPARADO PARA VERIFICAR ESTOS DATOS ____________________");
      Serial.println("MOSTRANDO EL CONTENIDO DEL ARCHIVO HELLO.TXT");
      readFile(SPIFFS, "/hello.txt");
      GRABAR_MATRIZ();
      ESP.restart();  //SE REALIZA UN RESETEO AUTOMATICO PARA CARGAR EL SISTEMA CON LOS NUEVOS DATOS
    }
  }

  if (STATUS_PAGE == 56) {
  
    STATUS_PAGE = 0;

    NUEVOMAC[0] = GOBIERNO.SUMERIO_MAC[RESTO][0];
    NUEVOMAC[1] = GOBIERNO.SUMERIO_MAC[RESTO][1];
    NUEVOMAC[2] = GOBIERNO.SUMERIO_MAC[RESTO][2];
    NUEVOMAC[3] = GOBIERNO.SUMERIO_MAC[RESTO][3];
    NUEVOMAC[4] = GOBIERNO.SUMERIO_MAC[RESTO][4];
    NUEVOMAC[5] = GOBIERNO.SUMERIO_MAC[RESTO][5];

    Serial.println("proceso de grabado en el chip ");
    Serial.printf("\n grabar %d:%d:%d:%d:%d:%d:", NUEVOMAC[0], NUEVOMAC[1], NUEVOMAC[2], NUEVOMAC[3], NUEVOMAC[4], NUEVOMAC[5]);
    GRABAR_CHIP();
    String S = SISTEMA;  // GetFile(SPIFFS, "/hello.txt");//
    datalog = String(S);
    Serial.print(datalog);
    //_________________________________________________________________
    //LIZADOANAR(datalog);
    //_________________________________________________________________
    appendFile(SPIFFS, "/hello.txt", datalog.c_str());

    Serial.println("_____________PREPARADO PARA VERIFICAR ESTOS DATOS ____________________");
    Serial.println("MOSTRANDO EL CONTENIDO DEL ARCHIVO HELLO.TXT");
    readFile(SPIFFS, "/hello.txt");
    GRABAR_MATRIZ();
    ESP.restart();  //SE REALIZA UN RESETEO AUTOMATICO PARA CARGAR EL SISTEMA CON LOS NUEVOS DATOS
  }
}
//*******************************************************************************************
void GRABAR_CHIP() {
  char linea;

  for (int j = 0; j <= 5; j++) {
    DECHEXA(NUEVOMAC[j], 16);
    SISTEMA = SISTEMA + ":";
  }
  SISTEMA = SISTEMA + '\n';
  Serial.println("______________________________________________________________________________________________________");
  Serial.println("mac addres  leida TRADUCIDA A HEXADECIMAL");
  Serial.println(SISTEMA);
  Serial.println("____________________LISTO__________________________________________________________________________________");
}
//___________________________________________________________________________________________________________________
void cliente_fuera() {
}
//________________________________________________________________________________________________________________________
void verifica_cliente() {
  //wifi_sta_list_t stationList;
  //LED=0; //INSERTAR ESTA CORRECION EN PORTON MAESTRO
  //ESP.wdtDisable();//Desabilita o SW WDT.

  esp_wifi_ap_get_sta_list(&stationList);
  NUN_CLIENT = stationList.num;
  Serial.printf("CANTIDAD DE CLIENTES CONECTADOS : %d \n", NUN_CLIENT);
  Serial.printf("LISTADO DE MACCADDRRESS CONECTADAS AL SISTEMA: \n");

  for (int j = 0; j < NUN_CLIENT; j++) {
    Serial.printf(" \n  ");
    Serial.printf(" cliente %d  ", j);
    wifi_sta_info_t station = stationList.sta[j];
    for (int i = 0; i < 6; i++) {
      Serial.printf(" mac %d  ", station.mac[i]);
      if (i < 5) {
        Serial.print(":");
      }
    }
  }

  if (NUN_CLIENT > MEMORIA_CLIENTES)  //SI EL NUMERO DE CLIENTES CONECTADOS ES MAYOR AL ANTERIOR ES PORQUE HAY UNA NUEVA CONEXION
  {
    MEMORIA_CLIENTES = NUN_CLIENT;
    //LISTADO_IP[NUN_CLIENT]=DIRECCION_IP;//SE VAN GRABANDO EL LISTADO DE LAS IP QUE INGRESAN
    Serial.println("nuevo cliente conectado :");
  }
}
//________________________________________________________________________________________________________________________
//________________________________________________________________________________________________________________________
void ABRE_PORTON()  //SE ENCARGA DE ABRIR EL PORTON
{
  //Vrdata=Vrdata+1;
  //  datalog = "Sensor 1: " + String(Vrdata) + "\n";
  //  Serial.println(datalog);
  //  appendFile(SPIFFS, "/hello.txt", datalog.c_str());
  //readFile(SPIFFS, "/hello.txt");
  digitalWrite(RELE, HIGH);  //SE HABILITA EL MOTOR
  delay(1000);
  digitalWrite(RELE, LOW);  //SE HABILITA EL MOTOR
  PROCESAR = 1;
  CUENTA_PASOS = 0;
}
//_______________________________________________________________________________________________________________________
//ESTA FUNCION ESTA RELACIONADA CON LA MANIPULACION DE DATOS DEL MODULO BLUETOOTH

String GetLine() {
  String S = "";
  if (SerialBT.available()) {
    char c = SerialBT.read();
    ;
    while (c != '\n')  //Hasta que el caracter sea intro
    {
      S = S + c;
      delay(25);
      c = SerialBT.read();
    }
    return (S + '\n');
  }
}
//_______________________________________________________________________________________________________


String NewLine() {
  String S = "";
  S = SINCEL;
  return (S + '\n');
}
//_______________________________________________________________________________________________________________________
//ESTA FUNCION ESTA RELACIONADA CON LA MANIPULACION DE DATOS PARA LEER LOS ARCHIVOS PASO A PASO

String GetFile(fs::FS &fs, const char *path) {

  File DATA = fs.open(path);


  //CUTRE[0] = DATA.read();

  DATA.close();
}
//_______________________________________________________________________________________________________
//ESTA FUNCION SE ENCARGA DE INTERPRETAR COMANDOS
void SHELL()  //ESTA FUNCION CONTIENE LA LINEA DE COMANDOS DEL SISTEMA ESTA EN PROCESO DE CONSTRUCCION SE PRETENDE REALIZAR UN SISTEMA OPERATIVO
{
}
//__________________________________________________________________________________________________
void ANALIZADOR(String bit)  //ESTA FUNCION ESTA DEDICADA A TOMAR UN CONJUNTO DE VALORES EN HEXADECIMAL Y LOS CONVIERTE A DECIMAL
{

  TABULADOR = 0;
  carril = 0;
  int OCTETO = 0;
  Serial.println("analizando caracter por caracter");

  while (bit[TABULADOR] != '\n') {
    Serial.print(bit[TABULADOR]);
    if (bit[TABULADOR] == '0') {
      Serial.println("=0");
      hexa[carril] = 0;
    }
    if (bit[TABULADOR] == '1') {
      Serial.println("=1");
      hexa[carril] = 1;
    }

    if (bit[TABULADOR] == '2') {
      Serial.println("=2");
      hexa[carril] = 2;
    }

    if (bit[TABULADOR] == '3') {
      Serial.println("=3");
      hexa[carril] = 3;
    }

    if (bit[TABULADOR] == '4') {
      Serial.println("=4");
      hexa[carril] = 4;
    }


    if (bit[TABULADOR] == '5') {
      Serial.println("=5");
      hexa[carril] = 5;
    }

    if (bit[TABULADOR] == '6') {
      Serial.println("=6");
      hexa[carril] = 6;
    }

    if (bit[TABULADOR] == '7') {
      Serial.println("=7");
      hexa[carril] = 7;
    }

    if (bit[TABULADOR] == '8') {
      Serial.println("=8");
      hexa[carril] = 8;
    }

    if (bit[TABULADOR] == '9') {
      Serial.println("=9");
      hexa[carril] = 9;
    }

    if (bit[TABULADOR] == 'A') {
      Serial.println("=10");
      hexa[carril] = 10;
    }

    if (bit[TABULADOR] == 'B') {
      Serial.println("=11");
      hexa[carril] = 11;
    }

    if (bit[TABULADOR] == 'C') {
      Serial.println("=12 ");
      hexa[carril] = 12;
    }

    if (bit[TABULADOR] == 'D') {
      Serial.println("=13 ");
      hexa[carril] = 13;
    }

    if (bit[TABULADOR] == 'E') {
      Serial.println("=14 ");
      hexa[carril] = 14;
    }

    if (bit[TABULADOR] == 'F') {
      Serial.println("=15 ");
      hexa[carril] = 15;
    }

    if (bit[TABULADOR] == ':') {

      Serial.printf("NUEVO OCTETO %d  \n", OCTETO);

      // Serial.print(carril);
      CONVERTIDOR();  //SE CONVIERTE EL NÚMERO HEXADECIMAL A DECIMAL
      decimal[OCTETO] = respuesta;
      OCTETO++;
      carril = -1;
    }
    carril++;
    TABULADOR++;
  }



  Serial.printf("_______________________________________________________________________________________\n");
  Serial.printf(" RESPUESTA FINAL \n");

  for (int j = 0; j < OCTETO; j++) {
    Serial.printf("DECIMAL  %d  \n", decimal[j]);
  }
  OCTETO = 0;
}
//_______________________________________________________________________________________________________________
void CONVERTIDOR()  //ESTA FUNCION SE ENCARGA DE CONVERTIR NUMEROS HEXADECIMALES EN DECIMALES TRABAJA EN CONJUNTO CON ANALIZADOR
{
  respuesta = 0;
  int potencia = 1;
  /*
Serial.println("datos almacenados en la variable hexa");
Serial.println(hexa[0]);
Serial.println(hexa[1]);
Serial.println("___________________________________________________________");

*/

  for (int i = 0; i < carril; i++) {
    Serial.println(hexa[i]);
  }


  Serial.println("datos almacenados en la variable hexa");
  for (int i = 0; i < carril; i++) {
    //Serial.println(hexa[carril-i]);
    respuesta = (hexa[carril - 1 - i] * potencia) + respuesta;
    potencia = potencia * 16;
  }
  Serial.printf("el proceso dio a la final esto: %d \n", respuesta);
  Serial.println("___________________________________________________________");

  //return(respuesta + '\n');
  /**********************************/
}
//______________________________________________________________________________________________________


//______________________________________________________________________________________________________________________________________
void INTERPRETAR() {


  if (K == "13") {
    //Serial.printf("=INICIO DE SALTO DE LINEA \n");
    CARRY++;
  }

  if (K == "10") {
    //Serial.printf("=INICIO DE SALTO DE LINEA \n");
    CARRY++;
  }

  if (CARRY == 3) {
    //Serial.printf("= SALTO DE LINEA \n");
    CARRY = 0;
  }

  if (K == "32") {
    //Serial.printf("=ESPACIO EN BLANCO \n");
    CARRY = 0;
  }

  if (K == "48") {
    //Serial.printf("=0 \n");
    CARRY = 0;
  }

  if (K == "49") {
    //Serial.printf("=1 \n");
    CARRY = 0;
  }

  if (K == "50") {
    //Serial.printf("=2 \n");
    CARRY = 0;
  }

  if (K == "51") {
    //Serial.printf("=3 \n");
    CARRY = 0;
  }


  if (K == "52") {
    //Serial.printf("=4 \n");
    CARRY = 0;
  }

  if (K == "53") {
    //Serial.printf("=5 \n");
    CARRY = 0;
  }

  if (K == "54") {
    //Serial.printf("=6 \n");
    CARRY = 0;
  }

  if (K == "55") {
    //Serial.printf("=7 \n");
    CARRY = 0;
  }

  if (K == "56") {
    //Serial.printf("=8 \n");
    CARRY = 0;
  }

  if (K == "57") {
    //Serial.printf("=9 \n");
    CARRY = 0;
  }


  if (K == "58") {
    //Serial.printf("=: \n");
    CARRY = 0;
  }

  //________________________________________________________________________

  if (K == "65") {
    //Serial.printf("=A \n");
    CARRY = 0;
  }

  if (K == "66") {
    //Serial.printf("=B \n");
    CARRY = 0;
  }

  if (K == "67") {
    //Serial.printf("=C \n");
    CARRY = 0;
  }

  if (K == "68") {
    //Serial.printf("=D \n");
    CARRY = 0;
  }

  if (K == "69") {
    //Serial.printf("=E \n");
    CARRY = 0;
  }

  if (K == "70") {
    //Serial.printf("=F \n");
    CARRY = 0;
  }

  if (K == "71") {
    //Serial.printf("=G \n");
    CARRY = 0;
  }

  //________________________________________________________________________

  if (K == "97") {
    //Serial.printf("=a \n");
    CARRY = 0;
  }

  if (K == "98") {
    //Serial.printf("=b \n");
    CARRY = 0;
  }

  if (K == "99") {
    //Serial.printf("=c \n");
    CARRY = 0;
  }

  if (K == "100") {
    //Serial.printf("=d \n");
    CARRY = 0;
  }

  if (K == "101") {
    //Serial.printf("=e \n");
    CARRY = 0;
  }

  if (K == "102") {
    //Serial.printf("=f \n");
    CARRY = 0;
  }
}
/*****************************************/
void CARGADOR(fs::FS &fs, const char *path)  //ESTA FUNCION SE VA A ENCARGAR DE LEER EL ARCHIVO LETRA POR LETRA Y DEFINIR EL GRABADO EN LA MATRIZ permiso
{
  //    Serial.printf("ESTA FUNCION SE VA A ENCARGAR DE LEER EL ARCHIVO LETRA POR LETRA Y DEFINIR EL GRABADO EN LA MATRIZ permiso \n");
  CUENTA_LETRA = 0;           //ESTA VARIABLE LLEVA EL CONTEO DE CUANTOS CARACTERES TIENE EL ARCHIVO QUE SE ESTA LEYENDO
  File file = fs.open(path);  //SE ABRE EL ARCHIVO SEGUN EL PATH O DIRECCIÓN COLOCADA

  if (!file || file.isDirectory()) {
    Serial.println("- FALLA EN LA LECTURA DE ARCHIVOS");
    return;
  }

  //Serial.println("- LECTURA DE ARCHIVO:");
  while (file.available()) {
    //Serial.write(file.read());
    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    delay(10);
    SINCEL = file.read();
    //Serial.printf("%s-------- ",SINCEL);
    CUENTA_LETRA++;
    LLEVO++;
    PUNTERO++;
    //Serial.print(SINCEL);//SI NECESITA MOSTRAR CADA ELEMENTO LEIDO DEL ARCHIVO
    //A PARTIR DE AQUI SE VA A ANALIZAR BIT A BIT EL ARCHIVO
    //*******************************************************************
    //SE LEE EL CODIGO ASCII DEL STRING SINCEL
    //____________________________________________________________

    if (SINCEL == "10") {
      //Serial.printf("=NUEVA LINEA \n");
      CARRY = 0;
      LINEA++;
      LAPIZ = 0;
      PUNTERO = 0;
    }

    //____________________________________________________________

    if (SINCEL == "32") {
      //Serial.printf("=ESPACIO EN BLANCO \n");
      LLEVO = LLEVO - 1;
      CARRY = 0;
    }

    //___________________________ACCION CUANDO CONSIGUE EL CODIGO ASCII DE LOS DOS PUNTOS_______________________________________________
    if (SINCEL == "58") {
      //Serial.printf("=: , GRABADO %d  Y  %d, LLEVO %d  TIENE QUE REALIZAR LA FUNCION DE CALCULO: EL CALCULO DIO: %d\n",AUXILIAR[1],AUXILIAR[2],LLEVO,((AUXILIAR[1]*16)+AUXILIAR[2]));
      CARRY = 0;
      PUNTERO = 0;

      permiso[LINEA][LAPIZ] = ((AUXILIAR[1] * 16) + AUXILIAR[2]);
      delay(10);  //se coloca un freno para evitar un desbordamiento de datos
      LAPIZ++;
    }
    //_______________________ANALIZANDO EL CODIGO ASCII DE LOS NÚMEROS___________________
    if (SINCEL == "48") {
      AUXILIAR[PUNTERO] = 0;
      //Serial.printf("=0 , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "49") {
      AUXILIAR[PUNTERO] = 1;
      //Serial.printf("=1 , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "50") {
      AUXILIAR[PUNTERO] = 2;
      //Serial.printf("=2 ,  GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "51") {
      AUXILIAR[PUNTERO] = 3;
      //Serial.printf("=3 , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "52") {
      AUXILIAR[PUNTERO] = 4;
      //Serial.printf("=4 , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "53") {
      AUXILIAR[PUNTERO] = 5;
      //Serial.printf("=5  GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "54") {
      AUXILIAR[PUNTERO] = 6;
      //Serial.printf("=6 , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "55") {
      AUXILIAR[PUNTERO] = 7;
      //Serial.printf("=7 , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "56") {
      AUXILIAR[PUNTERO] = 8;
      //Serial.printf("=8 , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "57") {
      AUXILIAR[PUNTERO] = 9;
      //Serial.printf("=9 , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }
    //**********************************************************************************************************************
    //________________________________ANALIZANDO CODIGO ASCII DE LAS LETRAS MAYÚSCULAS________________________________________
    if (SINCEL == "65") {
      AUXILIAR[PUNTERO] = 10;
      //Serial.printf("=A , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "66") {
      AUXILIAR[PUNTERO] = 11;
      //Serial.printf("=B , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "67") {
      AUXILIAR[PUNTERO] = 12;
      //Serial.printf("=C , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "68") {
      AUXILIAR[PUNTERO] = 13;
      //Serial.printf("=D , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "69") {
      AUXILIAR[PUNTERO] = 14;
      //Serial.printf("=E , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "70") {
      AUXILIAR[PUNTERO] = 15;
      //Serial.printf("=F , GRABADO %d, LLEVO %d\n",AUXILIAR[PUNTERO],LLEVO);
      CARRY = 0;
    }

    if (SINCEL == "71") {
      //Serial.printf("=G \n");
      CARRY = 0;
    }
    //_________________________________ANALIZANDO EL CODIGO ASCII DE LAS LETRAS MINÚSCULAS_______________________________________

    if (SINCEL == "97") {
      //Serial.printf("=a \n");
      AUXILIAR[PUNTERO] = 10;
      CARRY = 0;
    }

    if (SINCEL == "98") {
      //Serial.printf("=b \n");
      AUXILIAR[PUNTERO] = 11;
      CARRY = 0;
    }

    if (SINCEL == "99") {
      //Serial.printf("=c \n");
      AUXILIAR[PUNTERO] = 12;
      CARRY = 0;
    }

    if (SINCEL == "100") {
      //Serial.printf("=d \n");
      AUXILIAR[PUNTERO] = 13;
      CARRY = 0;
    }

    if (SINCEL == "101") {
      //Serial.printf("=e \n");
      AUXILIAR[PUNTERO] = 14;
      CARRY = 0;
    }

    if (SINCEL == "102") {
      //Serial.printf("=f \n");
      AUXILIAR[PUNTERO] = 15;
      CARRY = 0;
    }
  }
  file.close();
  //Serial.printf("cuenta letra dio:%d \n ",CUENTA_LETRA);
}
/*******************************************************************************************************************/
void HERON()  //SE ENCARGA DE MOVER EL MOTOR
{
  //digitalWrite(RELE,HIGH);//SE HABILITA EL MOTOR
  //esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
  //delay(30);
  esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
                         //digitalWrite(RELE,LOW);//SE HABILITA EL MOTOR
                         // ws.textAll(String(1));
  //digitalWrite(PIN_A,HIGH);
  //Serial.println(PROCESAR);
  if (PROCESAR == 1) {

    PASOS++;
    CUENTA_PASOS++;
    //Serial.println(PASOS);
    esp_task_wdt_reset();  //SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
    //delay(12);
    if (CUENTA_PASOS <= 150) {
      digitalWrite(PIN_A, HIGH);
      digitalWrite(PIN_B, LOW);
      //Serial.println("MOTOR EN RETROCESO");
      delay(15);
    }

    if ((CUENTA_PASOS > 150) && (CUENTA_PASOS < 250)) {
      digitalWrite(PIN_A, LOW);
      digitalWrite(PIN_B, LOW);
      //Serial.println("MOTOR DETENIDO");
      delay(15);
    }

    if ((CUENTA_PASOS >= 250) && (CUENTA_PASOS < LIMITE)) {
      digitalWrite(PIN_A, LOW);
      digitalWrite(PIN_B, HIGH);
      //Serial.println("MOTOR EN AVANCE");
      delay(15);
    }

    if (CUENTA_PASOS >= LIMITE) {
      PROCESAR = 0;
      CUENTA_PASOS = 0;
      PASOS = 0;
      digitalWrite(PIN_A, LOW);
      digitalWrite(PIN_B, LOW);
      //ESP.restart();  //SE REALIZA UN RESETEO AUTOMATICO PARA CARGAR EL SISTEMA CON LOS NUEVOS DATOS
    }
          
  }
  delay(8);
}
/**************************************************************************************/
void DECHEXA(int DIVIDENDO, int DIVISOR)  // ESTA FUNCION CONVIERTE DE NUEMEROS DECIMALES A  LA BASE DESEADA EN ESTE CASO HEXADECIMALES
{
  int taco = 0;
  char c;

  RESIDUO = DIVIDENDO;

  do {
    COCIENTE = 0;
    while (RESIDUO > DIVISOR) {
      RESIDUO = RESIDUO - DIVISOR;
      COCIENTE = COCIENTE + 1;
    }

    Serial.printf("\n los resultados dieron : \n residuo %d \n cociente %d", RESIDUO, COCIENTE);
    contenedor[taco] = RESIDUO;
    taco++;
    RESIDUO = COCIENTE;
  } while (COCIENTE > 0);

  Serial.println("resultado");
  for (int i = 0; i < taco; i++) {
    if (contenedor[taco - i - 1] == 0) {
      Serial.printf("0");
      SINCEL[i] = '0';
      c = '0';
    }

    if (contenedor[taco - i - 1] == 1) {
      Serial.printf("1");
      SINCEL[i] = '1';
      c = '1';
    }
    if (contenedor[taco - i - 1] == 2) {
      Serial.printf("2");
      SINCEL[i] = '2';
      c = '2';
    }
    if (contenedor[taco - i - 1] == 3) {
      Serial.printf("3");
      SINCEL[i] = '3';
      c = '3';
    }
    if (contenedor[taco - i - 1] == 4) {
      Serial.printf("4");
      SINCEL[i] = '4';
      c = '4';
    }
    if (contenedor[taco - i - 1] == 5) {
      Serial.printf("5");
      SINCEL[i] = '5';
      c = '5';
    }
    if (contenedor[taco - i - 1] == 6) {
      Serial.printf("6");
      SINCEL[i] = '6';
      c = '6';
    }
    if (contenedor[taco - i - 1] == 7) {
      Serial.printf("7");
      SINCEL[i] = '7';
      c = '7';
    }
    if (contenedor[taco - i - 1] == 8) {
      Serial.printf("8");
      SINCEL[i] = '8';
      c = '8';
    }
    if (contenedor[taco - i - 1] == 9) {
      Serial.printf("9");
      SINCEL[i] = '9';
      c = '9';
    }
    if (contenedor[taco - i - 1] == 10) {
      Serial.printf("A");
      SINCEL[i] = 'A';
      c = 'A';
    }
    if (contenedor[taco - i - 1] == 11) {
      Serial.printf("B");
      SINCEL[i] = 'B';
      c = 'B';
    }
    if (contenedor[taco - i - 1] == 12) {
      Serial.printf("C");
      SINCEL[i] = 'C';
      c = 'C';
    }
    if (contenedor[taco - i - 1] == 13) {
      Serial.printf("D");
      SINCEL[i] = 'D';
      c = 'D';
    }
    if (contenedor[taco - i - 1] == 14) {
      Serial.printf("E");
      SINCEL[i] = 'E';
      c = 'E';
    }
    if (contenedor[taco - i - 1] == 15) {
      Serial.printf("F");
      SINCEL[i] = 'F';
      c = 'F';
    }
    SISTEMA = SISTEMA + c;
  }
  //return( S + ':') ;
}
/*******************************************/
void ALEATORIO() {

  TIEMPO_NUEVO = SEGUNDOS;
  //Serial.println(" ");
  //Serial.printf(" el valor del tiempo antiguo es: %d  y el nuevo tiempo es %d",TIEMPO_NUEVO,TIEMPO_ANTIGUO);

  if (TIEMPO_NUEVO - TIEMPO_ANTIGUO >= 5) {
    TIEMPO_NUEVO = SEGUNDOS;
    TIEMPO_ANTIGUO = SEGUNDOS;
    RANDOM();
  }

  //esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
  ws.textAll(String("ALE1"));
  delay(180);
  ws.textAll(String(copa1));
  delay(180);

  ws.textAll(String("ALE2"));
  delay(180);
  ws.textAll(String(copa2));
  delay(180);

  ws.textAll(String("ALE3"));
  delay(180);
  ws.textAll(String(copa3));
  delay(180);

  ws.textAll(String("ALE4"));
  delay(180);
  ws.textAll(String(copa4));
  delay(180);

  ws.textAll(String("ALE5"));
  delay(180);
  ws.textAll(String(copa5));
  delay(180);

  ws.textAll(String("ALE6"));
  delay(180);
  ws.textAll(String(copa6));
  delay(180);
}
/*******************************************/
void SEND_MAC() {
  //esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
  ws.textAll(String("MAC1"));
  delay(180);
  ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][0]));
  copa7 = GOBIERNO.SUMERIO_MAC[RESTO][0];
  delay(180);

  //esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
  ws.textAll(String("MAC2"));
  delay(180);
  ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][1]));
  copa8 = GOBIERNO.SUMERIO_MAC[RESTO][1];
  delay(180);

  //esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
  ws.textAll(String("MAC3"));
  delay(180);
  ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][2]));
  copa9 = GOBIERNO.SUMERIO_MAC[RESTO][2];
  delay(180);

  //esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
  ws.textAll(String("MAC4"));
  delay(180);
  ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][3]));
  copa10 = GOBIERNO.SUMERIO_MAC[RESTO][3];
  delay(180);

  //esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
  ws.textAll(String("MAC5"));
  delay(180);
  ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][4]));
  copa11 = GOBIERNO.SUMERIO_MAC[RESTO][4];
  delay(180);
  //esp_task_wdt_reset();//SE RESETEA EL WATCHDOG TIMER ANTES DE QUE ESTE EXPIRE
  ws.textAll(String("MAC6"));
  delay(180);
  ws.textAll(String(GOBIERNO.SUMERIO_MAC[RESTO][5]));
  copa12 = GOBIERNO.SUMERIO_MAC[RESTO][5];
  delay(180);
}
//***************************************************************************
void GRABAR_MATRIZ() {
  for (int i = 0; i <= LINEA; i++) {
    for (int j = 0; j <= 5; j++) {
      Serial.printf("%d ", permiso[i][j]);
    }
    Serial.println("");
  }
}
//****************************************************************************************
void RELOG() {
  MILISEGUNDOS = MILISEGUNDOS + 1;
  if (MILISEGUNDOS == 500) {
    MILISEGUNDOS = 0;
    SEGUNDOS = SEGUNDOS + 1;
    if (SEGUNDOS == 60) {
      SEGUNDOS = 0;
      MINUTOS = MINUTOS + 1;
      if (MINUTOS == 60) {
        MINUTOS = 0;
        HORAS = HORAS + 1;
        if (HORAS == 24) {
          HORAS = 0;
          DIAS = DIAS + 1;
        }
      }
    }
  }
  //Serial.printf(" %d  HORA:%d MINUTO:%d SEGUNDOS:%d",MILISEGUNDOS,HORAS,MINUTOS,SEGUNDOS);
}
//***************************************************************************************************************************************
void CALCULO() {
  if ((((copa1 * copa6 * copa11) + (copa5 * copa10 * copa3) + (copa2 * copa7 * copa9)) - ((copa9 * copa6 * copa3) + (copa10 * copa7 * copa1) + (copa5 * copa2 * copa11))) != 0) {
    DETERMINANTE = ((copa1 * copa6 * copa11) + (copa5 * copa10 * copa3) + (copa2 * copa7 * copa9)) - ((copa9 * copa6 * copa3) + (copa10 * copa7 * copa1) + (copa5 * copa2 * copa11));
    Serial.printf("\n EL DETERMINANTE DEBERIA SER: %f", DETERMINANTE);
    RESPUESTA1 = (((copa4 * copa6 * copa11) + (copa8 * copa10 * copa3) + (copa12 * copa2 * copa7)) - ((copa12 * copa6 * copa3) + (copa8 * copa2 * copa11) + (copa10 * copa7 * copa4))) / DETERMINANTE;
    Serial.printf("\n PRIMERA SOLUCION : %f", RESPUESTA1);

    RESPUESTA2 = (((copa1 * copa8 * copa11) + (copa5 * copa12 * copa3) + (copa9 * copa4 * copa7)) - ((copa9 * copa8 * copa3) + (copa1 * copa12 * copa7) + (copa5 * copa4 * copa11))) / DETERMINANTE;
    Serial.printf("\n SEGUNDA SOLUCION : %f", RESPUESTA2);
  }
}
//***************************************************************************************************************************************
void COMPROBAR() {
  //TOKEN = 0;

  if (RESPUESTA1 >= 0) {
    if ((RESPUESTA1 - dato1 <= 0.0001) && (RESPUESTA2 - dato2 <= 0.0001)) {
      Serial.printf("\nHA PASADO LA PRUEBA OBTIENE UN TOKEN DE ACCESO");
      TOKEN = 2;
    }
  }

  if (RESPUESTA1 < 0) {
    if ((RESPUESTA1 - dato1 >= -0.0001) && (RESPUESTA2 - dato2 >= -0.0001)) {
      Serial.printf("\nHA PASADO LA PRUEBA OBTIENE UN TOKEN DE ACCESO");
      TOKEN = 2;
    }
  }

  if (TOKEN == 0) {
    Serial.printf("\n NEGADO EL ACCESO A LA PROXIMA PAGINA WEB ");
  }

  Serial.printf("\n EL VALOR DEL TOKEN ESTA EN :%d \n",TOKEN);

/*
  server.on("/REGISTRO", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (TOKEN == 2) {
      request->send_P(200, "text/html", REGISTRAR, processor);
    }

    else {
      request->send_P(200, "text/html", NEGADO, processor);
      TOKEN = 0;
    }

    STATUS_PAGE = 5;
  });
*/
}
//***************************************************************************************************************************************
void NMAP()//ESTA FUNCION MUESTRA TODOS LOS EQUIPOS CONECTADOS AL SISTEMA CON SUS RESPECTIVAS DIRECCIONES IP MAC ADDRES Y CARACTERISTICAS DE PERMISO
{
Serial.printf("\n __________________NMAP MOSTRANDO LAS CONEXIONES DEL SISTEMA______________________\n");
Serial.printf("CANTIDAD DE MAQUINAS CONECTADAS= %d \n",NUN_CLIENT);
      for(int C=0;C<NUN_CLIENT;C++)
      {
      Serial.printf("MAQUINA %d DIRECCION ip %s  DIRECCION MAC: %d:%d:%d:%d:%d:%d   ACCESO TIPO :%d \n",C+1,GOBIERNO.SUMERIO_IP[C],GOBIERNO.SUMERIO_MAC[C][0],GOBIERNO.SUMERIO_MAC[C][1],GOBIERNO.SUMERIO_MAC[C][2],GOBIERNO.SUMERIO_MAC[C][3],GOBIERNO.SUMERIO_MAC[C][4],GOBIERNO.SUMERIO_MAC[C][5],GOBIERNO.SUMERIO_ACCESO[C]);
      }
Serial.printf("ULTIMA SOLICITUD FUE DE LA MAQUINA %d= %s  PARA LA PAGINA %d\n",RESTO,GOBIERNO.SUMERIO_IP[RESTO],STATUS_PAGE);
}
//*******************************************************************************************************************************************
void RANDOM()
{
  copa1 = random(1, 255);
  copa2 = random(1, 255);
  copa3 = random(1, 255);
  copa4 = random(1, 255);
  copa5 = random(1, 255);
  copa6 = random(1, 255);
}



