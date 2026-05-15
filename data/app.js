// This is the file responsable for the front part of the project
// The page is devided in two tabs, one for monitoring the session and one for controlling it remotely
// Monitoring the session means setting the timmer and stopping it physically (from the esp32 project)
// Remote controlling means setting the timmer and stopping it from the remote tab (from the phone for example)


// All theese quotes can be changed and addapted to each person
const quotes = [
  "Disciplina inseamna sa alegi intre ceea ce vrei acum si ceea ce vrei cel mai mult!!",
  "Succesul nu vine din motivatie. Vine din ceea ce faci atunci cand nu ai chef!!",
  "Telefonul iti poate distruge ore intregi. Concentrarea iti poate construi ani intregi!!",
  "Fiecare minut petrecut distrat este un minut furat din viitorul tau!!",
  "Greu la invatatura, usor in lupta!!",
  "Concentreaza-te pe cine devii, nu pe cat de greu este momentul!!",
  "Daca nu iti controlezi atentia, altcineva o va face pentru tine!!",
  "Sunt sigura ca vei reusi sa faci asta!!"
];

let lastQuoteIdx  = -1;
let lastState     = null;
let quoteInterval = null;


// The format is - MM:SS 
function fmt(ms) {
  if (ms <= 0) return "00:00";
  const s = Math.floor(ms / 1000);
  const m = Math.floor(s / 60);
  return String(m).padStart(2, '0') + ':' + String(s % 60).padStart(2, '0');
}

// Fade quote in/out and replace text
function setQuote(text) {
  const el = document.getElementById('quote');
  el.classList.remove('visible');
  el.classList.add('hidden');
  setTimeout(() => {
    el.textContent = text;
    el.classList.remove('hidden');
    el.classList.add('visible');
  }, 500);
}

// Pick a random quote each time
function randomQuote() {
  let idx;
  do { idx = Math.floor(Math.random() * quotes.length); }
  while (idx === lastQuoteIdx);
  lastQuoteIdx = idx;
  return quotes[idx];
}

// Show a feedback message on the remote tab
function showMsg(text, type) {
  const el = document.getElementById('remote-msg');
  el.textContent  = text;
  el.className    = 'remote-msg ' + type;
  el.classList.remove('hidden');
  setTimeout(() => el.classList.add('hidden'), 3000); // Hide after 3 seconds
}

// The two tabs we have
function switchTab(name) {
  document.getElementById('tab-monitor').classList.toggle('hidden', name !== 'monitor');
  document.getElementById('tab-remote').classList.toggle('hidden',  name !== 'remote');

  document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
  document.querySelector(`.tab[onclick="switchTab('${name}')"]`).classList.add('active');
}

// Some remote actions

// Start a session remotely from the IP address of the esp32 project
function remoteStart() {
  const val = parseInt(document.getElementById('remote-minutes').value);
  if (!val || val < 1 || val > 180) {
    showMsg('Enter a valid time (1–180 min)', 'err');
    return;
  }
  fetch('/cmd?action=start&minutes=' + val)
    .then(r => r.json())
    .then(d => {
      if (d.ok) showMsg('Session started!', 'ok');
      else      showMsg('Could not start — ' + (d.reason || 'unknown error'), 'err');
    })
    .catch(() => showMsg('Connection error', 'err'));
}

// Take a break remotely from the IP address of the esp32 project
function remoteBreak() { 
  const val = parseInt(document.getElementById('break-minutes').value);
  if (!val || val < 1 || val > 30) {
    showMsg('Enter a valid break (1–30 min)', 'err');
    return;
  }
  fetch('/cmd?action=break&minutes=' + val)
    .then(r => r.json())
    .then(d => {
      if (d.ok) showMsg('Break started!', 'ok');
      else      showMsg('Could not start break — ' + (d.reason || ''), 'err');
    })
    .catch(() => showMsg('Connection error', 'err'));
}

// Stop/cancel session remotely from the IP address of the esp32 project
function remoteStop() {
  fetch('/cmd?action=stop')
    .then(r => r.json())
    .then(d => {
      if (d.ok) showMsg('Session stopped.', 'ok');
      else      showMsg('Could not stop — ' + (d.reason || ''), 'err');
    })
    .catch(() => showMsg('Connection error', 'err'));
}

// UI for the remote tab 
function updateRemote(d) {
  const inputBlock   = document.getElementById('remote-input-block');
  const activeBlock  = document.getElementById('remote-active-block');
  const ctrlBlock    = document.getElementById('remote-controls-block');
  const breakSection = document.getElementById('break-section');
  const remLabel     = document.getElementById('remote-state-label');
  const remTimer     = document.getElementById('remote-countdown');
  const remProgress  = document.getElementById('remote-progress');

  const isIdle = d.state === 'SETUP' || d.state === 'SUMMARY';

  inputBlock.classList.toggle('hidden', !isIdle);

  activeBlock.classList.toggle('hidden', isIdle);
  ctrlBlock.classList.toggle('hidden',   isIdle);

  if (!isIdle) {
    remLabel.className = 'label';
    switch (d.state) {
      case 'STUDYING':
        remLabel.textContent = 'STUDYING';
        remLabel.classList.add('studying');
        remTimer.textContent = fmt(d.remainingMs);
        const pct = d.studyDurationMs > 0
          ? Math.round(((d.studyDurationMs - d.remainingMs) / d.studyDurationMs) * 100)
          : 0;
        remProgress.style.width = pct + '%';
        remProgress.className   = 'progress-fill';
        break;

      case 'PAUSE_INPUT':
        remLabel.textContent = 'BREAK INPUT';
        remLabel.classList.add('paused');
        remTimer.textContent    = '--:--';
        remProgress.style.width = '0%';
        break;

      case 'PAUSED':
        remLabel.textContent = 'ON BREAK';
        remLabel.classList.add('paused');
        remTimer.textContent = fmt(d.pauseRemainingMs);
        const bPct = d.pauseDurationMs > 0
          ? Math.round(((d.pauseDurationMs - d.pauseRemainingMs) / d.pauseDurationMs) * 100)
          : 0;
        remProgress.style.width = bPct + '%';
        remProgress.className   = 'progress-fill paused';
        break;
    }

    breakSection.classList.toggle('hidden', d.state !== 'STUDYING');

    const breakBtn = document.getElementById('break-btn');
    if (d.rfidUsesLeft <= 0) {
      breakBtn.disabled   = true;
      breakBtn.textContent = 'NO BREAKS LEFT';
      breakBtn.style.opacity = '0.3';
    } else {
      breakBtn.disabled    = false;
      breakBtn.textContent = 'START BREAK (' + d.rfidUsesLeft + ' left)';
      breakBtn.style.opacity = '1';
    }
  }
}

// The monitor tab 
function updateMonitor(d) {
  const label    = document.getElementById('state-label');
  const timer    = document.getElementById('countdown');
  const fill     = document.getElementById('progress-fill');
  const bLeft    = document.getElementById('breaks-left');
  const bUsed    = document.getElementById('breaks-used');
  const bMins    = document.getElementById('break-mins');

  // Handle quote rotation when state changes
  if (d.state !== lastState) {
    clearInterval(quoteInterval);
    lastState = d.state;
    if (d.state === 'STUDYING') {
      setQuote(randomQuote());
      quoteInterval = setInterval(() => setQuote(randomQuote()), 30000);
    } else if (d.state === 'PAUSED' || d.state === 'PAUSE_INPUT') {
      setQuote("Take a real break. Step away.");
    } else if (d.state === 'SUMMARY') {
      const mins = Math.round((d.studyElapsedMs || 0) / 60000);
      setQuote("Session done. You studied " + mins + " minutes.");
    } else {
      setQuote("Start a session on the device or remote tab.");
    }
  }

  // Status label 
  // Theese are the main statuses and the logic is build around them, but they can be changed and addapted to each person
  label.className = 'label';
  switch (d.state) {
    case 'STUDYING':    label.textContent = 'STUDYING';    label.classList.add('studying'); break;
    case 'PAUSED':      label.textContent = 'ON BREAK';    label.classList.add('paused');   break;
    case 'PAUSE_INPUT': label.textContent = 'BREAK INPUT'; label.classList.add('paused');   break;
    case 'SUMMARY':     label.textContent = 'DONE';        label.classList.add('done');     break;
    default:            label.textContent = 'IDLE'; break;
  }

  // Countdown + progress bar
  timer.className = 'timer';
  fill.className  = 'progress-fill';

  if (d.state === 'STUDYING') {
    timer.textContent = fmt(d.remainingMs);
    const pct = d.studyDurationMs > 0
      ? Math.round(((d.studyDurationMs - d.remainingMs) / d.studyDurationMs) * 100) : 0;
    fill.style.width = pct + '%';

  } else if (d.state === 'PAUSED' || d.state === 'PAUSE_INPUT') {
    timer.textContent = fmt(d.pauseRemainingMs || 0);
    timer.classList.add('paused');
    fill.classList.add('paused');
    const bPct = d.pauseDurationMs > 0
      ? Math.round(((d.pauseDurationMs - (d.pauseRemainingMs || 0)) / d.pauseDurationMs) * 100) : 0;
    fill.style.width = bPct + '%';

  } else if (d.state === 'SUMMARY') {
    timer.textContent = '00:00';
    timer.classList.add('done');
    fill.classList.add('done');
    fill.style.width  = '100%';

  } else {
    timer.textContent = '--:--';
    fill.style.width  = '0%';
  }

  // Stats for UI display
  bLeft.textContent = d.rfidUsesLeft  ?? 3;
  bUsed.textContent = d.rfidUsesTotal ?? 0;
  bMins.textContent = Math.round((d.totalPauseMs || 0) / 60000);
}

// Fetch data from the server every 2 seconds and update the UI accordingly
function fetchData() {
  fetch('/data')
    .then(r => r.json())
    .then(d => {
      updateMonitor(d);
      updateRemote(d);
    })
    .catch(() => {}); // Silently ignore connection errors!!
}

fetchData();
setInterval(fetchData, 2000);