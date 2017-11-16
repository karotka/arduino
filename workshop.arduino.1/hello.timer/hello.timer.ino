void setTimer1() {
    // initialize timer1
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;               // value for compare

    OCR1A = 31250;            // compare match register 16MHz/256/2Hz
    TCCR1B |= (1 << WGM12);   // CTC mode (Clear Timer on Compare)
    TCCR1B |= (1 << CS12);    // 256 prescaler
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
}

ISR(TIMER1_COMPA_vect) {  // timer compare interrupt service routine
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void setup() {
    // initialize digital pin LED_BUILTIN as an output
    pinMode(LED_BUILTIN, OUTPUT);

    noInterrupts();           // disable all interrupts
    setTimer1();
    interrupts();             // enable all interrupts
}

// the loop function runs over and over again forever
void loop() {
}
