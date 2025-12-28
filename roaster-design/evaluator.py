# evaluator.py (starter)
import sys, yaml, math
import numpy as np
import matplotlib.pyplot as plt

def load_spec(path):
    with open(path,'r') as f:
        return yaml.safe_load(f)

def heater_checks(spec):
    V = spec['heater']['voltage']
    P = spec['heater']['power_w']
    I = P / V
    ssr = spec['ssr']['rating_a']
    fuse = spec['fuse']['recommended_a']
    print(f"Heater: {P} W @ {V} V -> I = {I:.2f} A")
    print(f"SSR rating: {ssr} A, recommended fuse: {fuse} A")
    if ssr < I*1.5:
        print("WARNING: SSR rating seems low vs expected current (apply margin)")
    return I

def psu_checks(spec, heater_current):
    vmot_v = spec['psu']['vmot_v']
    vmot_a = spec['psu']['vmot_a']
    fan_a = spec['psu']['fan_12v_a']
    logic_a = spec['psu']['logic_5v_a']
    motor_count = 2 # drum + valve
    stepper_a = spec['stepper']['current_a']
    motor_power_est = motor_count * stepper_a * vmot_v * 0.25 # rough
    print(f"VMOT supply {vmot_v} V @ {vmot_a} A; estimated motor load ~{motor_power_est:.1f} W")
    if vmot_a < stepper_a * motor_count * 0.6:
        print("WARNING: VMOT current may be low for peak currents; increase headroom.")
    return

def valve_steps_calc(spec):
    steps_per_rev = 200
    micro = spec['stepper']['microstep']
    gear = spec['stepper']['gearbox_ratio']
    steps_90 = int(steps_per_rev * micro * gear * 0.25)
    print(f"Valve 90° steps = {steps_90} (motor steps {steps_per_rev}, micro {micro}, gear {gear})")
    return steps_90

def torque_check(spec):
    motor_torque_kgcm = spec['stepper'].get('holding_torque_kgcm', None)
    if motor_torque_kgcm is None:
        print("No motor torque provided.")
        return
    motor_torque_nm = motor_torque_kgcm * 0.0980665
    gear = spec['stepper']['gearbox_ratio']
    out_torque = motor_torque_nm * gear
    needed = spec['valve']['required_torque_nm']
    print(f"Motor torque (Nm): {motor_torque_nm:.3f}, gearbox x{gear} -> output {out_torque:.3f} Nm. Valve need {needed} Nm")
    if out_torque < needed*1.5:
        print("WARNING: torque margin low; increase gearbox or motor torque.")

def thermal_simulation(spec):
    bean_mass = spec['drum']['bean_mass_g'] / 1000.0
    C_beans = 1260
    M_effective = 0.35 + 0.8*bean_mass
    Ce = C_beans * bean_mass + 5.0
    P = spec['heater']['power_w']
    k_loss = 10.0
    dt = 0.5
    tmax = 600
    Ta = 25.0
    T = Ta
    times = []
    temps = []
    for t in np.arange(0, tmax, dt):
        Q = P * 0.5
        dT = (Q - k_loss*(T - Ta)) / (Ce)
        T += dT*dt
        times.append(t)
        temps.append(T)
    plt.plot(times, temps)
    plt.xlabel('s'); plt.ylabel('°C'); plt.title('Simple roast sim (50% avg power)')
    plt.grid(True)
    plt.savefig('thermal_sim.png')
    print("Thermal sim saved to thermal_sim.png (very crude model).")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 evaluator.py design_spec.yaml")
        sys.exit(1)
    spec = load_spec(sys.argv[1])
    heater_I = heater_checks(spec)
    psu_checks(spec, heater_I)
    steps = valve_steps_calc(spec)
    torque_check(spec)
    thermal_simulation(spec)
    print("Evaluation done. Review warnings and thermal_sim.png")

if __name__ == '__main__':
    main()
