(() => {
  'use strict';
  const gestures = {
    neutral: { heading: 'Start from your center.', instruction: 'Hold the controller comfortably and keep it still during startup calibration. Remember that position: it is your neutral.', sensor: 'ACCELEROMETER + GYROSCOPE', result: 'After calibration, returning here stops steering.', action: 'HOLD STILL', response: 'Ship stays centered' },
    left: { heading: 'Lean left. Hold. Return.', instruction: 'From neutral, gently lean the controller to your left. Hold a small tilt to steer left, then bring it back to neutral to stop. Keep the movement slow and smooth.', sensor: 'ACCELEROMETER / TILT', result: 'The held angle gives the game a steering command.', action: 'GENTLE LEFT TILT', response: 'Ship moves left, then stops at neutral' },
    right: { heading: 'Lean right. Hold. Return.', instruction: 'From neutral, gently lean the controller to your right. Hold a small tilt to steer right, then bring it back to neutral to stop. Keep the same grip and orientation.', sensor: 'ACCELEROMETER / TILT', result: 'Returning to your calibrated center stops steering.', action: 'GENTLE RIGHT TILT', response: 'Ship moves right, then stops at neutral' },
    auto: { heading: 'Shots launch automatically.', instruction: 'Keep your usual grip. Tilt left or right to line the ship up with an enemy; firing needs no extra hand movement.', sensor: 'AUTOMATIC GAME ACTION', result: 'A shot launches after the firing interval when the previous projectile is gone.', action: 'HOLD AND AIM', response: 'Projectile launches automatically' }
  };
  const demo = document.querySelector('.gesture-demo');
  const buttons = [...document.querySelectorAll('[data-gesture]')];
  const fields = { heading: 'gesture-heading', instruction: 'gesture-instruction', sensor: 'gesture-sensor', result: 'gesture-result', action: 'visual-action', response: 'visual-response' };
  buttons.forEach(button => button.addEventListener('click', () => {
    const mode = button.dataset.gesture;
    buttons.forEach(item => item.setAttribute('aria-pressed', String(item === button)));
    demo.dataset.mode = mode;
    Object.entries(fields).forEach(([key, id]) => { document.getElementById(id).textContent = gestures[mode][key]; });
  }));
  const toggle = document.getElementById('animation-toggle');
  const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)');
  function setPaused(paused) {
    demo.classList.toggle('is-paused', paused);
    toggle.setAttribute('aria-pressed', String(paused));
    toggle.textContent = paused ? 'Play motion' : 'Pause motion';
  }
  setPaused(reducedMotion.matches);
  toggle.addEventListener('click', () => setPaused(!demo.classList.contains('is-paused')));
  reducedMotion.addEventListener('change', event => setPaused(event.matches));
  const checks = [...document.querySelectorAll('.drill-list input')];
  checks.forEach(check => check.addEventListener('change', () => {
    const count = checks.filter(input => input.checked).length;
    document.getElementById('drill-progress').textContent = count === checks.length ? '4 of 4 moves practiced — ready for your first run.' : `${count} of 4 moves practiced`;
  }));
})();
