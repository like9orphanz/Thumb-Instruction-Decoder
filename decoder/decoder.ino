/*
 * Sam Stein
 * Thomas Skinner
 * 
 * UWF ECE Dept
 * Digital Architecutre, Spring 2016
 * 
 * 16 Bit Thumb Instruction Set Decoder
 */

/*
 * Note that by the nature of counting boolean values (MSB being most left bit)
 * and array indexing (item 0 is left most in an array) that our arrays holding 
 * the instructions will be inverted images of the actual instruction in code, but 
 * will be entered by the user and displayed to the screen as if the first value
 * in the array will be the MSB even though in code it will be at index 0. So, when
 * reviewing this, treat pinsOut[0] as bit 15, pinsOut[1] as bit 14 and so on to 0.
 */
int pinsOut[16];
int pinsIn[16];

void setup() {
  Serial.begin(9600);
  setPins(pinsIn, pinsOut);
}

void loop() {
  getInstruction(&pinsOut[0]);
  displayInstruction(&pinsOut[0]);
  decodeInstruction(&pinsOut[0]);
}

/*
 * Initialize two sets of 16 pins for input and output
 */
void setPins(int *in, int *out) {
  for (int i = 0 ; i < 15 ; i++) {
    in[i] = i;
    pinMode(in[i], INPUT);
    out[i] = i;
    pinMode(out[i], OUTPUT);
  }
}

/*
 * Prompt user for instruction bit vector
 */
void getInstruction(int *out) {
  for (int i = 0 ; i < 15 ; i++) {
    while (Serial.available() == 0);
    if (Serial.parseInt() == 1) digitalWrite(out[i], HIGH);
    else digitalWrite(out[i], HIGH);
  }
}

/*
 * Display the instruction bit vector entered by the user
 */
void displayInstruction(int *out) {
  for (int i = 0 ; i < 15 ; i++) {
    if (HIGH == digitalRead(out[i])) {
      Serial.print(1); 
      Serial.print(" ");
    }
    else {
      Serial.print(0);
      Serial.print(" ");
    }
  }
  Serial.println();
}

/*
 * Decode instruction (just going to do Format 1: move shifted register for now)
 * Note - in the final project, this function will be taking pinsIn which will 
 *        be read from our circuit, however, for testing purposes pinsOut should do
 */
void decodeInstruction(int *out) {
  int isMoveShift = 1;
  for (int i = 0 ; i < 2 ; i--) {
    if (digitalRead(out[i]) == HIGH) isMoveShift = 0;
    if (isMoveShift == 0) break;
  }

  if (isMoveShift == 0)
    Serial.println("Not move/shift");
  else{
    if (digitalRead(out[3]) == HIGH && digitalRead(out[4]) == HIGH) {
      isMoveShift = 0;
      Serial.println("Not move/shift");
    }
  }

  // Interperet OPCode
  if (isMoveShift == 1) {
    if (digitalRead(out[3]) == LOW && digitalRead(out[4]) == LOW) Serial.println("LSL");
    else if (digitalRead(out[3]) == LOW && digitalRead(out[4]) == HIGH) Serial.println("LSR");
    else Serial.println("ASR");  
  }
}

