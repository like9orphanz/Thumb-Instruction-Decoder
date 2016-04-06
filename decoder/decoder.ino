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
  for ( int i = 0; i < 16; i++ )
  {
    digitalWrite(inPins[i], HIGH);
  }
  delay(400);
  for ( int i = 0; i < 16; i++ )
  {
    digitalWrite(inPins[i], LOW);
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
    longBranch ( in );
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
      exit(-1);
  }
  Serial.print(rd);
  Serial.print(", R");
  Serial.print(rs);
  Serial.print(", #");
  Serial.println(offset);
}

void immMCAS ( int *in )
{
  int offset8 = 0;
  int op = bit(1)*in[12] + bit(0)*in[11];
  int rd = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];
  for (int i = 7 ; i > -1 ; i--) offset8 = offset8 + bit(i)*in[i];

  switch (op)
  {
    case 0:
      Serial.print("MOV R");
    break;
    case 1:
      Serial.print("CMP R");
    break;  
    case 2:
      Serial.print("ADD R");
    break;
    case 3:
      Serial.print("SUB R");
    break;
    default:
      Serial.println("Error in decoding move/compare/add/subtract immediate");
      exit(-1);
  }
  
    Serial.print(rd);
    Serial.print(", ");
    Serial.println(offset8);
}

void aluOps ( int *in )
{
  int op = bit(3)*in[9] + bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rs = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];
  
  switch (op)
  {
    case 0:
      Serial.print("AND R");
    break;
    case 1:
      Serial.print("EOR R");
    break;
    case 2:
      Serial.print("LSL R");
    break;
    case 3:
      Serial.print("LSR R");
    break;
    case 4:
      Serial.print("ASR R");
    break;
    case 5:
      Serial.print("ADC R");
    break;
    case 6:
      Serial.print("SBC R");
    break;
    case 7:
      Serial.print("ROR R");
    break;
    case 8:
      Serial.print("TST R");
    break;
    case 9:
      Serial.print("NEG R");
    break;
    default:
      Serial.println("Error decoding aluOps");
      exit(-1);
  }

  Serial.print(rd);
  Serial.print(", R");
  Serial.println(rs);  
}

void hiRegOps ( int *in )
{
  int op = bit(1)*in[9] + bit(0)*in[8];
  int rs = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];

  switch (op)
  {
    case 0:
      Serial.print("ADD R");
    break;
    case 1:
      Serial.print("CMP R");
    break;
    case 2:
      Serial.print("MOV R");
    break;
    case 3:
      Serial.print("BX R");
    break;
    default:
      Serial.println("Error in decoding Hi register operations/branch exchange");
      exit(-1);
  }

  Serial.print(rd);
  Serial.print(", ");
  Serial.print("R");
  Serial.println(rs);
}

void PCRelLoad ( int *in )
{
  int word8 = 0;
  int rd = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];
  for (int i = 7 ; i > -1 ; i--) word8 = word8 + bit(i)*in[i];

  Serial.print("LDR R");
  Serial.print(rd);
  Serial.print(", [PC, #");
  Serial.print(word8);
  Serial.println("]");
}

void loadStoreRegOffset ( int *in )
{
  int lb = bit(1)*in[11] + bit(0)*in[10];
  int ro = bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rb = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];

  switch (lb)
  {
    case 0:
      Serial.print("STR R");
    break;
    case 1:
      Serial.print("STRB R");
    break;
    case 2:
      Serial.print("LDR R");
    break;
    case 3:
      Serial.print("LDRB R");
    break;
    default:
      Serial.println("Error decoding load/store with register offset");
      exit(-1);
  }

  Serial.print(rd);
  Serial.print(", [R");
  Serial.print(rb);
  Serial.print(", R");
  Serial.print(ro);
  Serial.println("]");
  
}

void loadStoreSignExt ( int *in )
{
  int hs = bit(1)*in[10] + bit(0)*in[11];
  int ro = bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rb = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];

  switch (hs)
  {
    case 0:
      Serial.print("STRH R");
    break;
    case 1:
      Serial.print("LDRH R");
    break;
    case 2:
      Serial.print("LDSB R");
    break;
    case 3:
      Serial.print("LDSH R");
    break;
    default:
      Serial.println("Error decoding load/store sign-extended byte/halfword");
      exit (-1);
  }

  Serial.print(rd);
  Serial.print(", [R");
  Serial.print(rb);
  Serial.print(", R");
  Serial.print(ro);
  Serial.println("]");
}

void loadStoreImm ( int *in )
{
  int bl = bit(1)*in[12] + bit(0)*in[11];
  int offset5 = bit(4)*in[10] + bit(3)*in[9] + bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rb = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];

  switch (bl)
  {
    case 0:
      Serial.print("STR R");
    break;
    case 1:
      Serial.print("LDR R");
    break;
    case 2:
      Serial.print("STRB R");
    break;
    case 3:
      Serial.print("LDRB R");
    break;
    default:
      Serial.println("Error decoding load/store with immediate offset");
      exit (-1);
  }

  Serial.print(rd);
  Serial.print(", [R");
  Serial.print(rb);
  Serial.print("#");
  Serial.print(offset5);
  Serial.println("]");
}

void loadStoreHalf ( int *in )
{
  int offset5 = bit(4)*in[10] + bit(3)*in[9] + bit(2)*in[8] + bit(1)*in[7] + bit(0)*in[6];
  int rb = bit(2)*in[5] + bit(1)*in[4] + bit(0)*in[3];
  int rd = bit(2)*in[2] + bit(1)*in[1] + bit(0)*in[0];

  if (in[11] == 0) Serial.print("STRH R");
  else Serial.print("LDRH R");

  Serial.print(rd);
  Serial.print(", [R");
  Serial.print(rb);
  Serial.print(", #");
  Serial.print(offset5);
  Serial.println("]");
}

void loadStoreSPRel ( int *in )
{
  int word8 = 0;
  int rd = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];
  for (int i = 7 ; i > -1 ; i--) word8 = word8 + bit(i)*in[i];

  if (in[11] == 0) Serial.print("STR R");
  else Serial.print("LDR R");

  Serial.print(rd);
  Serial.print(", [SP, #");
  Serial.print(word8);
  Serial.println("]");
}

void loadAddr ( int *in )
{
  int word8 = 0;
  int rd = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];
  for (int i = 7 ; i > -1 ; i--) word8 = word8 + bit(i)*in[i];

  Serial.print("R");
  Serial.print(rd);
  Serial.print(", ");
  if (in[11] == 0) Serial.print("PC, #");
  else Serial.print("SP, #");
  Serial.println(word8);
}

void addOffsetSP ( int *in )
{
  int sWord7 = 0;
  for (int i = 7 ; i > -1 ; i--) sWord7 = sWord7 + bit(i)*in[i];

  Serial.print("ADD SP, #");
  if (in[11] == 1) Serial.print("-");
  Serial.println(sWord7);
}

void pushPop ( int *in )
{
  int flag;
  int lr = bit(1)*in[11] + bit(0)*in[8];
  
  switch (lr)
  {
    case 0:
      Serial.print("PUSH {");
      for (int i = 7 ; i > -1 ; i--) {
        flag = 0;
        if (in[i] == 1) {
          Serial.print("R");
          Serial.print(i);
          for (int j = i ; j > -1 ; j--) if (in[j] == 1) flag = 1;
          if (flag == 1) Serial.print(",");
          else break;
        }
      }
      Serial.println("}");
    break;
    
    case 1:
      Serial.print("PUSH {");
      for (int i = 7 ; i > -1 ; i--) {
        flag = 0;
        if (in[i] == 1) {
          Serial.print("R");
          Serial.print(i);
          for (int j = i ; j > -1 ; j--) if (in[j] == 1) flag = 1;
          if (flag == 1) Serial.print(",");
          else break;
        }
      }
      Serial.println(" LR}");
    break;

    case 2:
      Serial.print("POP {");
      for (int i = 7 ; i > -1 ; i--) {
        flag = 0;
        if (in[i] == 1) {
          Serial.print("R");
          Serial.print(i);
          for (int j = i ; j > -1 ; j--) if (in[j] == 1) flag = 1;
          if (flag == 1) Serial.print(",");
          else break;
        }
      }
      Serial.println("}");
    break;

    case 3:
      Serial.print("POP {");
      for (int i = 7 ; i > -1 ; i--) {
        flag = 0;
        if (in[i] == 1) {
          Serial.print("R");
          Serial.print(i);
          for (int j = i ; j > -1 ; j--) if (in[j] == 1) flag = 1;
          if (flag == 1) Serial.print(",");
          else break;
        }
      }
      Serial.println(" PC}");
    break;
    default:
      Serial.println("Error decoding push/pop registers");
      exit (-1);  
  }
}

void multLoadStore ( int *in )
{
  int flag;
  int rb = bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];

  if (in[11] == 0) Serial.print("STMIA R");
  else Serial.print("LDMIA R");

  Serial.print(rb);
  Serial.print("!, {");
 
  for (int i = 7 ; i > -1 ; i--) {
    flag = 0;
    if (in[i] == 1) {
      Serial.print("R");
      Serial.print(i);
      for (int j = i ; j > -1 ; j--) if (in[j] == 1) flag = 1;
      if (flag == 1) Serial.print(",");
      else break;
    }
  }

  Serial.println("}");
}

void condBranch ( int *in )
{
  int cond = bit(3)*in[11] + bit(2)*in[10] + bit(1)*in[9] + bit(0)*in[8];

  switch (cond)
  {
    case 0:
      Serial.println("BEQ label");
    break;
    case 1:
      Serial.println("BNE label");
    break;
    case 2:
      Serial.println("BCS label");
    break;
    case 3:
      Serial.println("BCC label");
    break;
    case 4:
      Serial.println("BMI label");
    break;
    case 5:
      Serial.println("BPL label");
    break;
    case 6:
      Serial.println("BVS label");
    break;
    case 7:
      Serial.println("BVC label");
    break;
    case 8:
      Serial.println("BHI label");
    break;
    case 9:
      Serial.println("BLS label");
    break;
    case 10:
      Serial.println("BGE label");
    break;
    case 11:
      Serial.println("BLT label");
    break;
    case 12:
      Serial.println("BGT label");
    break;
    case 13:
      Serial.println("BLE label");
    break;
    default:
      Serial.println("Error decoding conditional branch");
      exit (-1);
  }
}

void softwareInt ( int *in )
{
  int value8 = 0;
  for (int i = 7 ; i > -1 ; i--) value8 = value8 + bit(i)*in[i];

  Serial.print("SWI ");
  Serial.println(value8);
}

void uncondBranch ( int *in )
{
  Serial.println("B label");
}

void longBranch ( int *in )
{
  Serial.println("BL label");
}

