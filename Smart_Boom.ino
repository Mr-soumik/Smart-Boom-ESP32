#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RELAY_PIN 18
#define LED_PIN 19
#define BUZZER_PIN 23
#define RELAY_ON LOW
#define RELAY_OFF HIGH

#define FIRE_MS 3000
#define DONE_MS 3500

const char* AP_SSID = "SmartBoom_TT360";
const char* AP_PASS = "128128128";

enum State { S_SAFE, S_ARMED, S_COUNT, S_FIRING, S_DONE };
const char* names[] = {"SAFE", "ARMED", "COUNTDOWN", "FIRING", "DONE"};

Adafruit_SSD1306 display(128, 64, &Wire, -1);
AsyncWebServer server(80);
DNSServer dns;
IPAddress apIP(192, 168, 4, 1);

volatile State state = S_SAFE;
volatile uint32_t stateStart = 0;
volatile uint16_t countSec = 10;
volatile uint16_t fired = 0;
volatile uint32_t buzzUntil = 0;
uint32_t lastFrame = 0;
float starX[24], starY[24], starS[24];

const char PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<meta name="theme-color" content="#07070f"><title>Smart Boom | TechTadka360</title>
<style>
:root{--c:#2dff8f}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent;user-select:none;-webkit-user-select:none}
body{margin:0;min-height:100vh;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif;color:#fff;background:radial-gradient(circle at 50% -10%,#3a1466 0,#12071f 45%,#05050b 100%);overflow-x:hidden}
body[data-s=SAFE]{--c:#2dff8f}
body[data-s=ARMED]{--c:#ffb800}
body[data-s=COUNTDOWN]{--c:#ff3b3b}
body[data-s=FIRING]{--c:#ff7a00}
body[data-s=DONE]{--c:#ffd84d}
canvas{position:fixed;inset:0;width:100%;height:100%;z-index:0;pointer-events:none}
.wrap{position:relative;z-index:1;max-width:460px;margin:0 auto;padding:16px 16px 30px}
.top{display:flex;justify-content:space-between;align-items:center}
.brand{font-weight:800;letter-spacing:2px;font-size:14px}
.brand b{color:#ffb800}
.chip{font-size:11px;font-weight:700;padding:6px 12px;border-radius:20px;border:1px solid}
.chip.on{color:#2dff8f;border-color:#2dff8f55;background:#2dff8f14}
.chip.off{color:#ff5c5c;border-color:#ff5c5c55;background:#ff5c5c14}
.hero{text-align:center;margin:22px 0 16px}
.eyebrow{font-size:11px;letter-spacing:5px;color:#ffb800;font-weight:700}
h1{margin:4px 0;font-size:40px;font-weight:900;letter-spacing:3px;background:linear-gradient(90deg,#ffb800,#ff3b3b,#ff00c8);-webkit-background-clip:text;background-clip:text;color:transparent;filter:drop-shadow(0 0 14px #ff3b3b66)}
.hero p{margin:0;font-size:13px;opacity:.65}
.card{background:rgba(255,255,255,.06);border:1px solid rgba(255,255,255,.12);backdrop-filter:blur(14px);-webkit-backdrop-filter:blur(14px);border-radius:28px;padding:22px 18px;box-shadow:0 20px 60px #0008}
.ringbox{position:relative;width:236px;height:236px;margin:0 auto 18px}
.ringbox svg{width:100%;height:100%;transform:rotate(-90deg)}
.ringbox circle{fill:none;stroke-width:10}
.ringbox .bg{stroke:rgba(255,255,255,.09)}
.ringbox .fg{stroke:var(--c);stroke-linecap:round;stroke-dasharray:653.45;stroke-dashoffset:0;transition:stroke-dashoffset .4s linear,stroke .3s;filter:drop-shadow(0 0 8px var(--c))}
.mid{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center;text-align:center}
#big{font-size:58px;font-weight:900;color:var(--c);text-shadow:0 0 24px var(--c);line-height:1}
#sub{margin-top:10px;font-size:12px;letter-spacing:1.5px;opacity:.75;max-width:150px}
body[data-s=COUNTDOWN] .ringbox{animation:pulse .5s infinite alternate}
body[data-s=FIRING] #big{animation:shake .08s infinite}
@keyframes pulse{to{transform:scale(1.04)}}
@keyframes shake{0%{transform:translate(2px,1px)}50%{transform:translate(-2px,-1px)}100%{transform:translate(1px,-2px)}}
.chips{display:flex;align-items:center;justify-content:center;gap:8px;margin-bottom:16px;font-size:12px;opacity:.95}
.chips span{opacity:.6;margin-right:4px}
.chips button{background:rgba(255,255,255,.08);color:#fff;border:1px solid rgba(255,255,255,.15);border-radius:14px;padding:7px 14px;font-weight:700;font-size:13px}
.chips button.on{background:#ffb800;color:#000;border-color:#ffb800}
.slide{position:relative;height:62px;border-radius:31px;background:rgba(255,255,255,.08);border:1px solid rgba(255,255,255,.14);overflow:hidden;text-align:center;line-height:60px;font-size:13px;font-weight:800;letter-spacing:3px;color:#ffffffaa}
.slide span{animation:glow 1.6s infinite}
@keyframes glow{50%{opacity:.4}}
.thumb{position:absolute;left:4px;top:4px;width:52px;height:52px;border-radius:50%;background:var(--c);color:#000;display:flex;align-items:center;justify-content:center;font-size:20px;touch-action:none;box-shadow:0 0 20px var(--c)}
.fire,.abort,.ghost{display:block;width:100%;border-radius:36px;font-weight:900;letter-spacing:3px;position:relative;overflow:hidden}
.fire{height:76px;border:none;background:linear-gradient(135deg,#ff3b3b,#ff7a00);color:#fff;font-size:20px;box-shadow:0 0 34px #ff3b3b88;animation:throb 1s infinite alternate}
@keyframes throb{to{box-shadow:0 0 54px #ff7a00cc}}
.fire .fill{position:absolute;left:0;top:0;bottom:0;width:0;background:rgba(255,255,255,.4)}
.fire.hold .fill{animation:fill 1.2s linear forwards}
@keyframes fill{to{width:100%}}
.fire b{position:relative}
.ghost{margin-top:12px;height:48px;background:transparent;border:1px solid rgba(255,255,255,.25);color:#fff;font-size:13px}
.abort{height:66px;background:#ff3b3b22;border:2px solid #ff3b3b;color:#ff6b6b;font-size:18px}
.hide{display:none!important}
.stats{display:flex;gap:10px;margin-top:14px}
.stats div{flex:1;text-align:center;padding:12px 6px;border-radius:18px;background:rgba(255,255,255,.05);border:1px solid rgba(255,255,255,.1);font-size:11px;opacity:.9}
.stats b{display:block;font-size:20px;color:#ffb800;margin-bottom:2px}
.tips{margin-top:14px;padding:14px 16px;border-radius:18px;background:#ffb80012;border:1px solid #ffb80044;font-size:12px;line-height:1.7;color:#ffe3a0}
.tips strong{letter-spacing:2px;color:#ffb800}
.foot{text-align:center;margin-top:18px;font-size:11px;opacity:.5}
</style></head><body data-s="SAFE">
<canvas id="fx"></canvas>
<div class="wrap">
<div class="top"><div class="brand">TECH<b>TADKA</b>360</div><div id="net" class="chip off">&#9679; CONNECTING</div></div>
<div class="hero"><div class="eyebrow">DIWALI EDITION</div><h1>SMART BOOM</h1><p>Remote cracker ignition system</p></div>
<div class="card">
<div class="ringbox"><svg viewBox="0 0 240 240"><circle class="bg" cx="120" cy="120" r="104"/><circle id="fg" class="fg" cx="120" cy="120" r="104"/></svg>
<div class="mid"><div id="big">SAFE</div><div id="sub">STANDBY</div></div></div>
<div id="timer" class="chips"><span>Countdown</span><button data-t="5">5s</button><button data-t="10" class="on">10s</button><button data-t="15">15s</button></div>
<div id="slide" class="slide"><div id="thumb" class="thumb">&#10148;</div><span>SLIDE TO ARM</span></div>
<button id="fire" class="fire hide"><i class="fill"></i><b>HOLD TO FIRE</b></button>
<button id="disarm" class="ghost hide">DISARM</button>
<button id="abort" class="abort hide">ABORT</button>
</div>
<div class="stats"><div><b id="cnt">0</b>BLASTS</div><div><b>OFFLINE</b>HOTSPOT MODE</div></div>
<div class="tips"><strong>SAFETY FIRST</strong><br>Stay 10+ metres away from the cracker.<br>Keep kids and pets far back.<br>Never touch the wire once armed.</div>
<div class="foot">Made with &#9829; by TechTadka360 &middot; Happy Diwali</div>
</div>
<script>
const $=id=>document.getElementById(id);
let st={s:'SAFE',ms:0,total:10,fired:0},T=10,prev='';
const C=653.45;
async function api(p){try{const r=await fetch(p,{cache:'no-store'});return await r.json()}catch(e){return null}}
const show=(id,v)=>$(id).classList.toggle('hide',!v);
function render(){
const s=st.s;document.body.dataset.s=s;
let frac=1,big=s,sub='';
if(s==='SAFE'){big='SAFE';sub='STANDBY &middot; SLIDE TO ARM'}
else if(s==='ARMED'){big='ARMED';sub='HOLD THE BUTTON TO FIRE'}
else if(s==='COUNTDOWN'){big=Math.ceil(st.ms/1000);sub='T-MINUS &middot; STAND BACK';frac=st.ms/(st.total*1000)}
else if(s==='FIRING'){big='FIRE!';sub='IGNITION LIVE'}
else{big='BOOM!';sub='HAPPY DIWALI!'}
$('big').textContent=big;$('sub').innerHTML=sub;
$('fg').style.strokeDashoffset=C*(1-Math.max(0,Math.min(1,frac)));
show('slide',s==='SAFE');show('fire',s==='ARMED');show('disarm',s==='ARMED');
show('abort',s==='COUNTDOWN'||s==='FIRING');show('timer',s==='SAFE'||s==='ARMED');
$('cnt').textContent=st.fired;
if(prev!==s){
if(s==='FIRING'){bigShow();if(navigator.vibrate)navigator.vibrate([200,80,400])}
if(s==='DONE')bigShow();
prev=s}
}
async function poll(){const d=await api('/status');
if(d){st=d;render();$('net').className='chip on';$('net').innerHTML='&#9679; CONNECTED'}
else{$('net').className='chip off';$('net').innerHTML='&#9679; LINK LOST'}}
setInterval(poll,400);poll();
document.querySelectorAll('#timer button').forEach(b=>b.onclick=()=>{T=+b.dataset.t;document.querySelectorAll('#timer button').forEach(x=>x.classList.remove('on'));b.classList.add('on')});
const th=$('thumb'),sl=$('slide');let drag=false,x0=0,max=0;
th.addEventListener('pointerdown',e=>{drag=true;x0=e.clientX;max=sl.clientWidth-th.clientWidth-8;th.style.transition='none';th.setPointerCapture(e.pointerId)});
th.addEventListener('pointermove',e=>{if(!drag)return;const d=Math.max(0,Math.min(max,e.clientX-x0));th.style.transform='translateX('+d+'px)';th.dataset.d=d});
async function endDrag(){if(!drag)return;drag=false;const d=+th.dataset.d||0;th.style.transition='transform .3s';th.style.transform='translateX(0)';th.dataset.d=0;
if(d>max*.9){const r=await api('/arm');if(r){st=r;render()}}}
th.addEventListener('pointerup',endDrag);th.addEventListener('pointercancel',endDrag);
const fb=$('fire');let ht=null;
fb.addEventListener('pointerdown',e=>{e.preventDefault();fb.classList.add('hold');ht=setTimeout(async()=>{fb.classList.remove('hold');ht=null;const r=await api('/fire?t='+T);if(r){st=r;render()}},1200)});
const he=()=>{if(ht){clearTimeout(ht);ht=null}fb.classList.remove('hold')};
['pointerup','pointerleave','pointercancel'].forEach(n=>fb.addEventListener(n,he));
document.addEventListener('contextmenu',e=>e.preventDefault());
$('disarm').onclick=async()=>{const r=await api('/disarm');if(r){st=r;render()}};
$('abort').onclick=async()=>{const r=await api('/abort');if(r){st=r;render()}};
const cv=$('fx'),g=cv.getContext('2d');let W,H,P=[];
function rs(){W=cv.width=innerWidth;H=cv.height=innerHeight}rs();addEventListener('resize',rs);
function burst(x,y,n,h){for(let i=0;i<n;i++){const a=Math.random()*6.283,s=Math.random()*5+1;P.push({x:x,y:y,vx:Math.cos(a)*s,vy:Math.sin(a)*s,l:1,h:h+Math.random()*40})}}
function bigShow(){for(let i=0;i<9;i++)setTimeout(()=>burst(Math.random()*W,Math.random()*H*.6,70,Math.random()*360),i*200)}
function frame(){g.globalCompositeOperation='destination-out';g.fillStyle='rgba(0,0,0,.2)';g.fillRect(0,0,W,H);g.globalCompositeOperation='lighter';
for(const p of P){p.x+=p.vx;p.y+=p.vy;p.vy+=.06;p.vx*=.985;p.l-=.012;g.fillStyle='hsla('+p.h+',100%,60%,'+Math.max(p.l,0)+')';g.fillRect(p.x,p.y,2.4,2.4)}
P=P.filter(p=>p.l>0);requestAnimationFrame(frame)}
frame();
function auto(){burst(Math.random()*W,Math.random()*H*.5,36,Math.random()*360);setTimeout(auto,st.s==='COUNTDOWN'?500:2200)}
auto();
</script></body></html>)rawliteral";

void setState(State s) {
  stateStart = millis();
  state = s;
}

void beep(uint16_t ms) {
  buzzUntil = millis() + ms;
}

void sendStatus(AsyncWebServerRequest* r) {
  int32_t ms = 0;
  if (state == S_COUNT) {
    ms = (int32_t)countSec * 1000 - (int32_t)(millis() - stateStart);
  }
  if (ms < 0) ms = 0;
  String j = "{\"s\":\"";
  j += names[state];
  j += "\",\"ms\":";
  j += ms;
  j += ",\"total\":";
  j += (int)countSec;
  j += ",\"fired\":";
  j += (int)fired;
  j += "}";
  AsyncWebServerResponse* res = r->beginResponse(200, "application/json", j);
  res->addHeader("Cache-Control", "no-store");
  r->send(res);
}

void drawStars() {
  for (int i = 0; i < 24; i++) {
    starX[i] -= starS[i];
    if (starX[i] < 0) {
      starX[i] = 127;
      starY[i] = random(0, 64);
    }
    display.drawPixel((int)starX[i], (int)starY[i], SSD1306_WHITE);
  }
}

void drawBomb(int cx, int cy, uint32_t now) {
  display.fillCircle(cx, cy, 13, SSD1306_WHITE);
  display.fillCircle(cx - 5, cy - 5, 3, SSD1306_BLACK);
  display.fillRect(cx - 4, cy - 17, 8, 5, SSD1306_WHITE);
  display.drawLine(cx, cy - 17, cx + 5, cy - 23, SSD1306_WHITE);
  display.drawLine(cx + 5, cy - 23, cx + 11, cy - 24, SSD1306_WHITE);
  int sx = cx + 11, sy = cy - 24;
  for (int i = 0; i < 5; i++) {
    float a = random(0, 628) / 100.0;
    int l = random(2, 6);
    display.drawLine(sx, sy, sx + cos(a) * l, sy + sin(a) * l, SSD1306_WHITE);
  }
  display.fillCircle(sx, sy, 1 + (now / 80) % 2, SSD1306_WHITE);
}

void drawBurst(int cx, int cy, int ph, int n) {
  if (ph < 0) return;
  if (ph < 12) display.drawCircle(cx, cy, ph * 3, SSD1306_WHITE);
  if (ph < 8) display.drawCircle(cx, cy, ph * 2, SSD1306_WHITE);
  for (int i = 0; i < n; i++) {
    if (ph > 22 && ((i + ph) & 1)) continue;
    float a = i * 2 * PI / n;
    float sp = 1.4 + (i % 4) * 0.6;
    float r = sp * ph;
    float t = r * 0.7;
    int x = cx + cos(a) * r;
    int y = cy + sin(a) * r + ph * ph * 0.015;
    int x2 = cx + cos(a) * t;
    int y2 = cy + sin(a) * t + ph * ph * 0.01;
    display.drawLine(x2, y2, x, y, SSD1306_WHITE);
  }
}

void drawSafe(uint32_t now) {
  drawStars();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("TECHTADKA360");
  if ((now / 500) % 2) display.fillCircle(122, 4, 2, SSD1306_WHITE);
  else display.drawCircle(122, 4, 2, SSD1306_WHITE);
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  drawBomb(30, 40, now);
  display.setTextSize(3);
  display.setCursor(56, 16);
  display.print("SAFE");
  display.setTextSize(1);
  display.setCursor(56, 44);
  display.print("READY TO ARM");
  display.setCursor(0, 56);
  int m = (now / 2500) % 4;
  if (m == 0) {
    display.print("WIFI SmartBoom_TT360");
  } else if (m == 1) {
    display.print("IP 192.168.4.1");
  } else if (m == 2) {
    display.print("PASS 128128128");
  } else {
    display.print("USERS ");
    display.print(WiFi.softAPgetStationNum());
    display.print("  BLASTS ");
    display.print((int)fired);
  }
}

void drawArmed(uint32_t now) {
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  if ((now / 150) % 2) display.drawRect(2, 2, 124, 60, SSD1306_WHITE);
  bool on = (now / 300) % 2;
  if (on) display.fillTriangle(30, 10, 8, 50, 52, 50, SSD1306_WHITE);
  else display.drawTriangle(30, 10, 8, 50, 52, 50, SSD1306_WHITE);
  display.setTextSize(3);
  display.setTextColor(on ? SSD1306_BLACK : SSD1306_WHITE);
  display.setCursor(21, 22);
  display.print("!");
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(62, 12);
  display.print("ARMED");
  display.setTextSize(1);
  display.setCursor(62, 34);
  display.print("AWAITING");
  display.setCursor(62, 44);
  display.print("FIRE CMD");
  display.fillRect(((now / 10) % 150) - 14, 54, 14, 2, SSD1306_WHITE);
}

void drawCount(uint32_t now, uint32_t el) {
  int32_t rem = (int32_t)countSec * 1000 - (int32_t)el;
  if (rem < 0) rem = 0;
  int sec = (rem + 999) / 1000;
  float frac = (float)rem / ((float)countSec * 1000.0);
  int cx = 32, cy = 34;
  for (int i = 0; i < 40; i++) {
    float a = -PI / 2 + i * 2 * PI / 40;
    if (i <= frac * 40) {
      display.drawLine(cx + cos(a) * 23, cy + sin(a) * 23, cx + cos(a) * 28, cy + sin(a) * 28, SSD1306_WHITE);
    } else {
      display.drawPixel(cx + cos(a) * 27, cy + sin(a) * 27, SSD1306_WHITE);
    }
  }
  int digits = sec >= 10 ? 2 : 1;
  display.setTextSize(3);
  display.setCursor(cx - digits * 9, cy - 12);
  display.print(sec);
  display.setTextSize(1);
  display.setCursor(66, 2);
  display.print("T-MINUS");
  bool vis = sec > 3 || ((now / 150) % 2);
  if (vis) {
    display.setTextSize(2);
    display.setCursor(66, 16);
    display.print("STAND");
    display.setCursor(66, 34);
    display.print("BACK!");
  }
  display.setTextSize(1);
  display.setCursor(66, 54);
  display.print("ABORT: APP");
}

void drawFiring(uint32_t now, uint32_t el) {
  if (el < 100) {
    display.fillScreen(SSD1306_WHITE);
    return;
  }
  int t = el / 33;
  int cyc = t / 30;
  int ph = t % 30;
  int cx = 64 + (cyc * 47) % 50 - 25;
  int cy = 28 + (cyc * 29) % 20 - 10;
  drawBurst(cx, cy, ph, 28);
  drawBurst(128 - cx, cy + 6, ph - 8, 20);
  display.drawRect(0, 0, 128, 4, SSD1306_WHITE);
  display.fillRect(0, 0, 128 * el / FIRE_MS, 4, SSD1306_WHITE);
  display.fillRect(30, 46, 68, 18, SSD1306_BLACK);
  display.setTextSize(2);
  display.setCursor(34 + random(-2, 3), 48 + random(-1, 2));
  display.print("FIRE!");
}

void drawDone(uint32_t now, uint32_t el) {
  int sx = 0, sy = 0;
  if (el < 700) {
    sx = random(-3, 4);
    sy = random(-2, 3);
  }
  for (int i = 0; i < 10; i++) {
    uint32_t h = ((now / 140) + i) * 2654435761u;
    int x = 4 + h % 120;
    int y = 4 + (h >> 9) % 56;
    display.drawFastHLine(x - 2, y, 5, SSD1306_WHITE);
    display.drawFastVLine(x, y - 2, 5, SSD1306_WHITE);
  }
  display.fillRect(10, 8, 108, 30, SSD1306_BLACK);
  display.setTextSize(3);
  display.setCursor(19 + sx, 12 + sy);
  display.print("BOOM!");
  display.fillRect(24, 41, 80, 22, SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(28, 43);
  display.print("HAPPY DIWALI");
  display.setCursor(40, 54);
  display.print("BLASTS: ");
  display.print((int)fired);
}

void drawFrame(uint32_t now) {
  uint32_t el = now - stateStart;
  State s = state;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  if (s == S_SAFE) drawSafe(now);
  else if (s == S_ARMED) drawArmed(now);
  else if (s == S_COUNT) drawCount(now, el);
  else if (s == S_FIRING) drawFiring(now, el);
  else drawDone(now, el);
  display.display();
}

void bootAnim() {
  for (int f = 0; f < 110; f++) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    if (f < 45) {
      int y = 62 - f;
      display.fillRect(63, y, 2, 6, SSD1306_WHITE);
      for (int k = 0; k < 6; k++) {
        display.drawPixel(63 + random(-2, 4), y + 7 + k * 2 + random(0, 2), SSD1306_WHITE);
      }
    } else if (f < 80) {
      drawBurst(64, 16, f - 45, 28);
    }
    if (f >= 50) {
      display.fillRect(0, 22, 128, 20, SSD1306_BLACK);
      int n = min(10, (f - 50) / 2);
      display.setTextSize(2);
      display.setCursor(4, 24);
      display.print(String("SMART BOOM").substring(0, n));
    }
    if (f >= 80) {
      int w = min(128, (f - 80) * 6);
      display.drawFastHLine(64 - w / 2, 44, w, SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(34, 50);
      display.print("TechTadka360");
    }
    digitalWrite(BUZZER_PIN, (f >= 45 && f < 49) ? HIGH : LOW);
    display.display();
    delay(30);
  }
  digitalWrite(BUZZER_PIN, LOW);
}

void updateOutputs(uint32_t now, uint32_t el) {
  State s = state;
  digitalWrite(RELAY_PIN, s == S_FIRING ? RELAY_ON : RELAY_OFF);
  static int lastSec = -1;
  if (s == S_COUNT) {
    int32_t rem = (int32_t)countSec * 1000 - (int32_t)el;
    if (rem < 0) rem = 0;
    int sec = (rem + 999) / 1000;
    if (sec != lastSec) {
      lastSec = sec;
      beep(sec <= 3 ? 250 : 80);
    }
  } else {
    lastSec = -1;
  }
  bool buz = now < buzzUntil || s == S_FIRING || (s == S_DONE && (el < 150 || (el > 300 && el < 450)));
  digitalWrite(BUZZER_PIN, buz ? HIGH : LOW);
  bool led;
  if (s == S_SAFE) led = true;
  else if (s == S_ARMED) led = (now / 250) % 2;
  else if (s == S_COUNT) led = (now / 60) % 2;
  else if (s == S_FIRING) led = (now / 40) % 2;
  else led = (now / 120) % 2;
  digitalWrite(LED_PIN, led ? HIGH : LOW);
}

void setup() {
  digitalWrite(RELAY_PIN, RELAY_OFF);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);
  for (int i = 0; i < 24; i++) {
    starX[i] = random(0, 128);
    starY[i] = random(0, 64);
    starS[i] = 0.3 + random(0, 10) / 10.0;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASS, 1, 0, 4);
  dns.start(53, "*", apIP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* r) {
    r->send_P(200, "text/html", PAGE);
  });
  server.on("/status", HTTP_GET, sendStatus);
  server.on("/arm", HTTP_GET, [](AsyncWebServerRequest* r) {
    if (state == S_SAFE) {
      setState(S_ARMED);
      beep(150);
    }
    sendStatus(r);
  });
  server.on("/disarm", HTTP_GET, [](AsyncWebServerRequest* r) {
    if (state == S_ARMED) {
      setState(S_SAFE);
      beep(80);
    }
    sendStatus(r);
  });
  server.on("/fire", HTTP_GET, [](AsyncWebServerRequest* r) {
    if (state == S_ARMED) {
      int t = 10;
      if (r->hasParam("t")) t = r->getParam("t")->value().toInt();
      if (t < 5) t = 5;
      if (t > 30) t = 30;
      countSec = t;
      setState(S_COUNT);
    }
    sendStatus(r);
  });
  server.on("/abort", HTTP_GET, [](AsyncWebServerRequest* r) {
    if (state == S_ARMED || state == S_COUNT || state == S_FIRING) {
      digitalWrite(RELAY_PIN, RELAY_OFF);
      setState(S_SAFE);
      beep(400);
    }
    sendStatus(r);
  });
  server.onNotFound([](AsyncWebServerRequest* r) {
    r->redirect("http://192.168.4.1/");
  });
  server.begin();

  bootAnim();
  setState(S_SAFE);
}

void loop() {
  dns.processNextRequest();
  uint32_t now = millis();
  uint32_t el = now - stateStart;
  State s = state;
  if (s == S_COUNT && el >= (uint32_t)countSec * 1000) {
    fired++;
    setState(S_FIRING);
  } else if (s == S_FIRING && el >= FIRE_MS) {
    setState(S_DONE);
  } else if (s == S_DONE && el >= DONE_MS) {
    setState(S_SAFE);
  }
  updateOutputs(now, millis() - stateStart);
  if (now - lastFrame >= 33) {
    lastFrame = now;
    drawFrame(now);
  }
}
