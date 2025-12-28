# Coffee Roaster (500 g) — Design Repository

This repository contains the design files, schematics, mechanical drawings, firmware, and evaluation scripts for a 500 g drum coffee roaster. The design supports both electric (SSR switched) and gas-fired burners; the current working design uses a stepper-actuated valve for gas modulation and a LIN 4118L stepper for drum drive.

Contents (roaster-design/):
- wiring-kicad-ready.md — KiCad-ready wiring notes and detailed pin mapping for Arduino, DM542 drivers, SSR, MOSFETs, thermocouples, and power supplies.
- mechanical-drawing-updated.md — Dimensioned mechanical drawing notes (drum, bearings, motor mount, jacket) and fabrication guidance.
- updated-wiring-and-control.md — Updated wiring and control notes for gas-fired roaster, including solenoid, igniter, flame detector, and stepper-actuated valve strategy.
- updated-mechanical-drawing-for-servo-valve.md — Mechanical notes for valve actuation (kept for reference; final design uses stepper-actuated valve).
- gas_valve_stepper_roaster.ino — Arduino sketch implementing drum and valve steppers, homing, ignition, PID->valve mapping, flame safety and E-stop logic.
- mechanical-drawing.svg — SVG 1:1 mechanical sketch (top & side views) for the drum, jacket, motor plate and pulley layout.
- evaluator.py — Local Python evaluation agent starter script (electrical checks, torque checks, simple thermal sim) to run in WSL/Windows.
- design_spec.yaml — Example design specification used by the evaluator agent.

Safety notice
--------------
This project involves mains electricity and gas. Do not connect or test the gas system without proper safety measures and, where required, professional inspection. Use certified components for gas and mains wiring if the device will be used long-term or indoors. The included firmware and designs are provided as-is; test, review, and adapt for your environment.

How to use these files
----------------------
- View Markdown files in VS Code or convert to PDF with Pandoc (WSL commands provided in the conversation).
- Use mechanical-drawing.svg in Inkscape to export to DXF for fabrication.
- Run evaluator.py with the included design_spec.yaml to get a basic report and thermal sim graph.
- Upload the Arduino sketch to an Uno/Nano (with appropriate wiring and safety hardware) after thoroughly reviewing and adjusting constants.

Contact & next steps
--------------------
If you'd like, I can also push KiCad schematic files (.sch), provide a DXF export of the SVG, or add a GitHub Actions workflow to run the evaluator automatically on commits.
