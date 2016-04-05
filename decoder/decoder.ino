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

int pins[16] = {
  22,23,24,25,
  26,27,28,29,
  30,31,32,33,
  34,35,36,37};

void setup() {
  setPins(pins);
  testLEDs(pins);
}

void loop() {
  Serial.begin(9600);
  getInstruction(pins);
  displayInstruction(pins);
  decodeInstruction(pins);
  Serial.end();
}

/*
 *initialize a set of pins for output
 */
void setPins(int *inPins) {
  for (int i = 0 ; i < 16 ; i++) {
    pinMode(inPins[i], OUTPUT);
  }
}

/*
 * Prompt user for instruction bit vector
 */
void getInstruction(int *inPins) {
  for (int i = 0 ; i < 16 ; i++) {
    while (Serial.available() == 0);
    char charRead = Serial.read();
    if ( charRead == '1')
    {
      digitalWrite(inPins[i], HIGH);
    }
    else
    {
      digitalWrite(inPins[i], LOW);
    }
  }
}

/*
 * Display the instruction bit vector entered by the user
 */
void displayInstruction(int *inPins) {
  for (int i = 0 ; i < 16 ; i++) {
    if (digitalRead(inPins[i]) == HIGH) {
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

void testLEDs ( int *inPins )
{
  for ( int i = 0; i < 16; i++ )
  {
    digitalWrite(inPins[i], HIGH);
    delay(150);
    digitalWrite(inPins[i], LOW);
    delay(10);
  }
}

/*
 * Decode instruction (just going to do Format 1: move shifted register for now)
 * Note - in the final project, this function will be taking pinsIn which will 
 *        be read from our circuit, however, for testing purposes pinsOut should do
 */
void decodeInstruction(int *inPins) {
  //reverse instruction from what was read so that we can process it human-like
  int in[16];
  for (int i = 0 ; i < 16 ; i++) {
    if (digitalRead(inPins[i]) == HIGH) {
      in[15-i] = HIGH;
    }
    else {
      in[15-i] = LOW;
    }
  }
  
  
  //determine the function
  int func = bit(2)*in[15] + bit(1)*in[14] + bit(0)*in[13];
  switch ( func )
  {
    case 0:
      //f 1 2
      f1thru2 ( in );
      break;
    case 1:
      //f 3
      f3 ( in );
      break;
    case 2:
      //f 4 5 6 7 8
      f4thru8( in );
      break;
    case 3:
      //f 9
      f9 ( in );
      break;
    case 4:
      //f 10 11
      f10thru11 ( in );
      break;
    case 5:
      //f 12 13 14
      f12thru14 ( in );
      break;
    case 6:
      //f 15 16 17
      f15thru17( in );
      break;
    case 7:
      //f 18 19
      f18thru19( in );
      break;
    deafult:
      Serial.println("An error occurred when finding the function.");
  }
}

void f1thru2 ( int *in )
{
  int test = bit(1)*in[12] + bit(0)*in[11];
  if ( test == 3 )
    addSub ( in );
  else
    moveShifted ( in );
}

void f3 ( int *in )
{
  immMCAS ( in );
}

void f4thru8 ( int *in )
{
  if ( !in[12] )
    if ( !in[11] )
      if ( !in[10] )
        aluOps ( in );
      else
        hiRegOps ( in );
    else
      PCRelLoad ( in );
  else
    if ( !in[9] )
      loadStoreRegOffset ( in );
    else
      loadStoreSignExt ( in );
}

void f9 ( int *in )
{
  loadStoreImm ( in );
}

void f10thru11 ( int *in )
{
  if ( !in[12] )
    loadStoreHalf ( in );
  else
    loadStoreSPRel ( in );
}

void f12thru14 ( int *in )
{
  Serial.println("made it to 5");
  if ( !in[12] )
    loadAddr ( in );
  else
    if ( !in[10] )
      addOffsetSP ( in );
    else
      pushPop ( in );
}

void f15thru17 ( int *in )
{
  if ( !in[12] )
    multLoadStore ( in );
  else
    if ( !( in[11] && in[10] ) )
      condBranch ( in );
    else
      softwareInt ( in );
}

void f18thru19 ( int *in )
{
  if ( !in[12] )
    uncondBranch ( in );
  else
    langBranch ( in );
}

void addSub ( int *in )
{
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];
  int rs = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rn_off = bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  if ( !in[9] )
    Serial.print("ADD ");
  else
    Serial.print("SUB ");
  Serial.print ( "R" );
  Serial.print ( rd );
  Serial.print ( ", R" );
  Serial.print ( rs );
  Serial.print ( ", " );
  if ( !in[10] )
    Serial.print ( "R" );
  else
    Serial.print( "#" );
  Serial.println ( rn_off );
}

void moveShifted ( int *in )
{
  int op = bit(1)*in[12] + bit(0)*in[11];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];
  int rs = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int offset = bit(4)*in[10] + bit(3)*in[9] + bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  switch ( op )
  {
    case 0:
      Serial.print("LSL R");
      break;
    case 1:
      Serial.print("LSR R");
      break;
    case 2:
      Serial.print("ASR R");
      break;
    default:
      Serial.println("problem decoding in addSub()");
  }
  Serial.print(rd);
  Serial.print(", R");
  Serial.print(rs);
  Serial.print(", #");
  Serial.println(offset);
}

void immMCAS ( int *in )
{
  
}

void aluOps ( int *in )
{
  
}

void hiRegOps ( int *in )
{
  
}

void PCRelLoad ( int *in )
{
  
}
void loadStoreRegOffset ( int *in )
{
  
}

void loadStoreSignExt ( int *in )
{
  
}

void loadStoreImm ( int *in )
{
  
}

void loadStoreHalf ( int *in )
{
  
}

void loadStoreSPRel ( int *in )
{
  
}

void loadAddr ( int *in )
{
  
}

void addOffsetSP ( int *in )
{
  
}

void pushPop ( int *in )
{
  
}

void multLoadStore ( int *in )
{
  
}

void condBranch ( int *in )
{
  
}

void softwareInt ( int *in )
{
  
}

void uncondBranch ( int *in )
{
  
}

void langBranch ( int *in )
{
  
}

