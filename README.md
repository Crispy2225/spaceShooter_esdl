# Flick & Fly sensor player guide

A responsive, dependency-free guide to playing the physical LPC1768 space shooter with an MPU6050 accelerometer and gyroscope.

The guide teaches calibration and neutral grip, accelerometer tilt for steering, quick downward wrist flicks detected by the gyroscope for firing, the physical start/restart button, and gameplay. Selectable animated illustrations demonstrate the gestures. A checklist provides drills to perform on the actual device, and troubleshooting explains drift, missed flicks, and unintended movement.

## Preview

Open `index.html` in a browser, or run:

```sh
python3 -m http.server 5173
```

Then visit http://localhost:5173.

## Files

- `index.html`: instructions, sensor illustrations, physical practice checklist, and troubleshooting.
- `styles.css`: responsive layout and gesture animations.
- `app.js`: demonstration selection, motion pause, and checklist progress.

This is an instructional website, not a live sensor connection or browser game. Animations are illustrative and do not display measured sensor readings. The confirmed firing gesture is a quick downward wrist flick, tipping the front edge of the controller toward the floor. Left/right tilt is reserved for steering. The sensor mounting orientation, register-axis mapping, thresholds, and cooldown duration remain firmware choices. No numeric hardware settings are invented.

All artwork is local SVG and CSS. No build step, remote assets, or package installation is required.
