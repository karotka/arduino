void setup() {
    Serial.begin(115200); // Other baud rates can be used...

    // initialize digital pin LED_BUILTIN as an output
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("My Sketch has started");
}

// the loop function runs over and over again forever
void loop() {
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH 5V)
    Serial.print("Set digital write on ");
    Serial.println(HIGH, DEC);
    delay(1000);                     // wait for a second

    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (GND)
    Serial.print("Set digital write on ");
    Serial.println(LOW, DEC);
    delay(1000);                     // wait for a second
}
