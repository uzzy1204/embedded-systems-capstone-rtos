#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

const char* ssid = "TTUguest";
const char* password = "maskedraiders";

const String USERNAME = "admin";
const String PASSWORD = "1234";
bool isLoggedIn = false;

ESP8266WebServer server(80);

Adafruit_INA219 ina219_fan(0x40);
Adafruit_INA219 ina219_lamp(0x41);

const int LAMP_RELAY = D5;
bool lampRelayOn = false;

// ========== DASHBOARD PAGE ==========
String dashboardPage() {
  float fan_v   = ina219_fan.getBusVoltage_V();
  float fan_i_A = ina219_fan.getCurrent_mA() / 1000.0;
  float fan_p_W = fan_v * fan_i_A;

  float lamp_v   = ina219_lamp.getBusVoltage_V();
  float lamp_i_A = ina219_lamp.getCurrent_mA() / 1000.0;
  float lamp_p_W = lamp_v * lamp_i_A;

  String html = R"=====(
  <html>
  <head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <style>
      body {
        background: linear-gradient(135deg, #0a0f24, #111827);
        font-family: Arial, sans-serif;
        color: white; text-align: center;
        margin: 0; padding: 0;
      }
      h1 {
        margin-top: 20px;
        font-size: 26px;
        letter-spacing: 1px;
      }
      .nav {
        margin-top: 8px;
        font-size: 14px;
      }
      .nav a {
        color: #38bdf8;
        margin: 0 8px;
        text-decoration: none;
      }
      .card {
        background: rgba(255,255,255,0.05);
        padding: 18px;
        margin: 15px auto;
        width: 90%; max-width: 420px;
        border-radius: 15px;
        backdrop-filter: blur(6px);
        box-shadow: 0 0 20px rgba(0,0,0,0.4);
      }
      .icon { font-size: 32px; display: block; margin-bottom: 5px; }
      .label { font-size: 14px; opacity: 0.7; margin-bottom: 6px; }
      .row {
        display: flex;
        justify-content: space-between;
        margin: 2px 0;
        font-size: 14px;
      }
      .btn {
        padding: 10px 20px;
        margin: 8px;
        border: none;
        border-radius: 30px;
        font-size: 16px;
        cursor: pointer;
        color: white;
        transition: 0.2s;
      }
      .onBtn  { background: #22c55e; }
      .offBtn { background: #ef4444; }
      .btn:hover { transform: scale(1.05); opacity: 0.9; }
      .timestamp {
        margin-top: 10px;
        font-size: 13px;
        opacity: 0.6;
      }
      .status-dot {
        display: inline-block;
        width: 10px; height: 10px;
        border-radius: 999px;
        margin-left: 6px;
      }
      .on-dot  { background: #22c55e; }
      .off-dot { background: #ef4444; }
      a.logout {
        color: #38bdf8;
        display: block;
        margin-top: 15px;
        font-size: 16px;
      }
    </style>
  </head>
  <body>
    <h1>&#127968; Smart Home Energy Dashboard</h1>
    <div class='nav'>
      <a href='/'>Overview</a> |
      <a href='/graphs'>Graphs</a>
    </div>

    <div class='card'>
      <span class='icon'>&#127744;</span>
      <div class='label'>Fan (Monitored Only)</div>
      <div class='row'><span>Voltage:</span><span>)=====" + String(fan_v,3) + R"=====( V</span></div>
      <div class='row'><span>Current:</span><span>)=====" + String(fan_i_A,3) + R"=====( A</span></div>
      <div class='row'><span>Power:</span><span>)=====" + String(fan_p_W,3) + R"=====( W</span></div>
    </div>

    <div class='card'>
      <span class='icon'>&#128161;</span>
      <div class='label'>Lamp (Monitored & Controlled)</div>
      <div class='row'><span>Voltage:</span><span>)=====" + String(lamp_v,3) + R"=====( V</span></div>
      <div class='row'><span>Current:</span><span>)=====" + String(lamp_i_A,3) + R"=====( A</span></div>
      <div class='row'><span>Power:</span><span>)=====" + String(lamp_p_W,3) + R"=====( W</span></div>
      <div style='margin-top:8px;'>
        <span>Status: )=====" + String(lampRelayOn ? "ON" : "OFF") + R"=====( 
          <span class='status-dot )=====" + String(lampRelayOn ? "on-dot" : "off-dot") + R"=====('></span>
        </span>
      </div>
      <div style='margin-top:10px;'>
        <button class='btn onBtn' onclick="fetch('/relay?state=1')">Lamp ON</button>
        <button class='btn offBtn' onclick="fetch('/relay?state=0')">Lamp OFF</button>
      </div>
    </div>

    <div class='timestamp'>
      Last Updated: )=====" + String(millis()/1000) + R"=====( s since power-up
    </div>

    <a class='logout' href='/logout'>Logout</a>

    <script>
      setTimeout(()=>{ location.reload(); }, 2000);
    </script>
  </body>
  </html>
  )=====";

  return html;
}

// ========== GRAPHS PAGE ==========
String graphsPage() {
  String html = R"=====(
  <html>
  <head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <script src='https://cdn.jsdelivr.net/npm/chart.js'></script>
    <style>
      body {
        background: #020617;
        font-family: Arial, sans-serif;
        color: white;
        text-align: center;
        margin: 0; padding: 0;
      }
      h1 {
        margin-top: 20px;
        font-size: 24px;
      }
      .nav {
        margin-top: 8px;
        font-size: 14px;
      }
      .nav a {
        color: #38bdf8;
        margin: 0 8px;
        text-decoration: none;
      }
      .card {
        background: rgba(15,23,42,0.9);
        padding: 16px;
        margin: 15px auto;
        width: 95%; max-width: 480px;
        border-radius: 14px;
        box-shadow: 0 0 20px rgba(0,0,0,0.6);
      }
      canvas { max-width: 100%; height: 260px; }
      .note {
        font-size: 12px;
        opacity: 0.7;
        margin-top: 8px;
      }
    </style>
  </head>
  <body>
    <h1>&#128200; Power Averages (Every 30s)</h1>
    <div class='nav'>
      <a href='/'>Overview</a> |
      <a href='/graphs'>Graphs</a>
    </div>

    <div class='card'>
      <canvas id='powerChart'></canvas>
      <div class='note'>
        Each point is the average power of the last 30 seconds.<br>
        Fan and Lamp power (W) sampled from /status.
      </div>
    </div>

    <script>
      const ctx = document.getElementById('powerChart').getContext('2d');
      const powerChart = new Chart(ctx, {
        type: 'line',
        data: {
          labels: [],
          datasets: [
            {
              label: 'Fan Power (W)',
              data: [],
              borderColor: 'rgba(56,189,248,1)',
              backgroundColor: 'rgba(56,189,248,0.2)',
              tension: 0.2
            },
            {
              label: 'Lamp Power (W)',
              data: [],
              borderColor: 'rgba(251,191,36,1)',
              backgroundColor: 'rgba(251,191,36,0.2)',
              tension: 0.2
            }
          ]
        },
        options: {
          responsive: true,
          scales: {
            x: { title: { display: true, text: 'Time' } },
            y: { title: { display: true, text: 'Power (W)' }, beginAtZero: true }
          }
        }
      });

      let sumFan = 0, sumLamp = 0, sampleCount = 0;
      let lastAvgTime = Date.now();
      const pollIntervalMs = 2000;
      const avgWindowMs  = 30000;

      function pollStatus() {
        fetch('/status')
          .then(r => r.json())
          .then(data => {
            const fanP  = data.fan_p || 0;
            const lampP = data.lamp_p || 0;

            sumFan  += fanP;
            sumLamp += lampP;
            sampleCount++;

            const now = Date.now();
            if (now - lastAvgTime >= avgWindowMs && sampleCount > 0) {
              const avgFan  = sumFan / sampleCount;
              const avgLamp = sumLamp / sampleCount;

              const label = new Date().toLocaleTimeString();
              powerChart.data.labels.push(label);
              powerChart.data.datasets[0].data.push(avgFan.toFixed(3));
              powerChart.data.datasets[1].data.push(avgLamp.toFixed(3));
              powerChart.update();

              sumFan = 0;
              sumLamp = 0;
              sampleCount = 0;
              lastAvgTime = now;
            }
          })
          .catch(e => {
            console.log('Fetch error:', e);
          });
      }

      setInterval(pollStatus, pollIntervalMs);
    </script>
  </body>
  </html>
  )=====";

  return html;
}

// ========== LOGIN PAGE ==========
String loginPage() {
  String html = "<html><body style='text-align:center;font-family:Arial;background:#111;color:white'>";
  html += "<h2>Smart Home Login</h2>";
  html += "<form action='/login' method='POST'>";
  html += "<input name='user' placeholder='Username'><br>";
  html += "<input type='password' name='pass' placeholder='Password'><br><br>";
  html += "<button type='submit'>Login</button></form>";
  html += "</body></html>";
  return html;
}

// ========== ROUTES ==========
void handleRoot() {
  if (!isLoggedIn) server.send(200, "text/html", loginPage());
  else            server.send(200, "text/html", dashboardPage());
}

void handleGraphs() {
  if (!isLoggedIn) server.send(200, "text/html", loginPage());
  else             server.send(200, "text/html", graphsPage());
}

void handleLogin() {
  if (server.arg("user") == USERNAME && server.arg("pass") == PASSWORD) {
    isLoggedIn = true;
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(200, "text/html", "<p style='color:red'>Incorrect</p>" + loginPage());
  }
}

void handleLogout() {
  isLoggedIn = false;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleStatusJSON() {
  float fan_v   = ina219_fan.getBusVoltage_V();
  float fan_i_A = ina219_fan.getCurrent_mA() / 1000.0;
  float fan_p_W = fan_v * fan_i_A;

  float lamp_v   = ina219_lamp.getBusVoltage_V();
  float lamp_i_A = ina219_lamp.getCurrent_mA() / 1000.0;
  float lamp_p_W = lamp_v * lamp_i_A;

  String json = "{";
  json += "\"fan_v\":"   + String(fan_v, 3)   + ",";
  json += "\"fan_i\":"   + String(fan_i_A, 3) + ",";
  json += "\"fan_p\":"   + String(fan_p_W, 3) + ",";
  json += "\"lamp_v\":"  + String(lamp_v, 3)  + ",";
  json += "\"lamp_i\":"  + String(lamp_i_A, 3)+ ",";
  json += "\"lamp_p\":"  + String(lamp_p_W, 3)+ ",";
  json += "\"lamp_relay\":" + String(lampRelayOn ? 1 : 0);
  json += "}";
  server.send(200, "application/json", json);
}

void handleRelayControl() {
  if (!server.hasArg("state")) {
    server.send(400, "text/plain", "Missing state");
    return;
  }
  int st = server.arg("state").toInt();
  bool on = (st == 1);

  lampRelayOn = on;
  digitalWrite(LAMP_RELAY, on ? HIGH : LOW);  // active HIGH

  server.send(200, "text/plain", "OK");
}

// ========== SETUP / LOOP ==========
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Connecting to TTUguest...");

  if (!ina219_fan.begin()) {
    Serial.println("INA219 (FAN @0x40) not found!");
  } else {
    Serial.println("INA219 FAN OK (0x40)");
  }

  if (!ina219_lamp.begin()) {
    Serial.println("INA219 (LAMP @0x41) not found!");
  } else {
    Serial.println("INA219 LAMP OK (0x41)");
  }

  pinMode(LAMP_RELAY, OUTPUT);
  digitalWrite(LAMP_RELAY, LOW);  // OFF by default
  lampRelayOn = false;

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("SUCCESS: Connected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("FAILED: WiFi may be blocked/login portal required.");
  }

  server.on("/", handleRoot);
  server.on("/graphs", handleGraphs);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/logout", handleLogout);
  server.on("/status", HTTP_GET, handleStatusJSON);
  server.on("/relay",  HTTP_GET, handleRelayControl);

  server.on("/relay/on", []() {
    lampRelayOn = true;
    digitalWrite(LAMP_RELAY, HIGH);
    server.send(200, "text/plain", "Lamp ON (legacy)");
  });
  server.on("/relay/off", []() {
    lampRelayOn = false;
    digitalWrite(LAMP_RELAY, LOW);
    server.send(200, "text/plain", "Lamp OFF (legacy)");
  });

  server.begin();
  Serial.println("Web Server Started");
}

void loop() {
  server.handleClient();
}
