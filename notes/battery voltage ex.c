// Reading from the Battery Pin
voltValue = analogRead(A0);

// Calculate current voltage level
battery_volt = ((voltValue * battery_voltage_ref) / 4095) * 1000;

// Battery level expressed in percentage
battery_percentage = 100 * abs((battery_volt - minimal_voltage) / ((battery_voltage_ref * 1000) - minimal_voltage));

Serial.print(F("Battery: "));
Serial.print(battery_percentage);
Serial.println(F(" % "));

if (battery_volt <= minimal_voltage) {
    // LED Notification for low voltage detection
    lowBatteryWarning();
}

delay(2000);

// Going into Low Power for 20 seconds
LowPower.deepSleep(20000);
}

// Low battery indicator
void lowBatteryWarning() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1);
    digitalWrite(LED_BUILTIN, LOW);
    delay(999);
}
