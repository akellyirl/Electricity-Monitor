#include <Console.h>
#include <Temboo.h>
#include <Process.h>

/*** SUBSTITUTE YOUR VALUES BELOW: ***/

// Note that for additional security and reusability, you could
// use #define statements to specify these values in a .h file.

const String GOOGLE_USERNAME = "wolola@gmail.com";
const String GOOGLE_PASSWORD = "3e3we3w";
#define TEMBOO_ACCOUNT "aSSdazx"  // your Temboo account name 
#define TEMBOO_APP_KEY_NAME "myFirstApp"  // your Temboo app key name
#define TEMBOO_APP_KEY  "ccdfddd-uuuuu"  // your Temboo app key

// the title of the spreadsheet you want to send data to
// (Note that this must actually be the title of a Google spreadsheet
// that exists in your Google Drive/Docs account, and is configured
// as described above.)
const String SPREADSHEET_TITLE = "ElectricityUsage";

int r0, r1, r2, u0, u1, u2, v , i= 0;
unsigned long time;
float Arms, AMS, Prms, Ptot, vf = 0;
float CAL = 1.05;

Process date;                 // process used to get the date

void setup() {
  // initialize communication:
  Bridge.begin();
  Console.begin(); 
  time = millis();
  if (!date.running())  {
    date.begin("date");
    date.addParameter("+%T");
    date.run();
  }
}

void loop() {
  // send the value of analog input 0:
  r2 = r1;
  r1 = r0;
  r0 = analogRead(A0);
  u2 = u1;
  u1 = u0;
  // 0.5Hz to 200Hz Band Pass Filter
  u0 = 0.2929*(r0-r2) + 1.411*u1 -0.4142*u2;
  v = u0;

  // Calculate Mean-Square Current (Amps)
  AMS = 0.99*AMS +0.01*v*v;
  // Calculate Root-Mean-Square (Amps)
  Arms = sqrt(AMS);
  // Convert to RMS Power:
  // Multipy by 230V (rms)
  // 30*5/1024 is the accounts for the gain of the Current Transformer and ADC
  Prms = 230*30*Arms*5/1024*CAL;
  delay(1);
 
 // Gather data for 5 seconds 
  if (millis() - time > 5000)
  {
    // Print the RMS Power in the last 5 seconds to the Console
    Console.print("Prms = ");
    Console.println(Prms);
    if (i == 719) {
      // As we just gathered 1 hour's readings this in in kWh
      Ptot = Ptot/720;
        
      // we need a Process object to send a Choreo request to Temboo
      TembooChoreo AppendRowChoreo;
  
      // invoke the Temboo client
      // NOTE that the client must be reinvoked and repopulated with
      // appropriate arguments each time its run() method is called.
      AppendRowChoreo.begin();
      
      // set Temboo account credentials
      AppendRowChoreo.setAccountName(TEMBOO_ACCOUNT);
      AppendRowChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
      AppendRowChoreo.setAppKey(TEMBOO_APP_KEY);
      
      // identify the Temboo Library choreo to run (Google > Spreadsheets > AppendRow)
      AppendRowChoreo.setChoreo("/Library/Google/Spreadsheets/AppendRow");
      
      // set the required Choreo inputs
      // see https://www.temboo.com/library/Library/Google/Spreadsheets/AppendRow/ 
      // for complete details about the inputs for this Choreo
      
      // your Google username (usually your email address)
      AppendRowChoreo.addInput("Username", GOOGLE_USERNAME);
  
      // your Google account password
      AppendRowChoreo.addInput("Password", GOOGLE_PASSWORD);
  
      // the title of the spreadsheet you want to append to
      // NOTE: substitute your own value, retaining the "SpreadsheetTitle:" prefix.
      AppendRowChoreo.addInput("SpreadsheetTitle", SPREADSHEET_TITLE);
      
      // restart the date process:
      if (!date.running())  {
        date.begin("date");
        date.addParameter("+%T");
        date.run();
      }
      // convert the time and sensor values to a comma separated string
      String timeString = date.readString(); 
      String rowData(timeString);
      rowData += ",";
      rowData += Ptot;
  
      // add the RowData input item
      AppendRowChoreo.addInput("RowData", rowData);
  
      // run the Choreo and wait for the results
      // The return code (returnCode) will indicate success or failure 
      unsigned int returnCode = AppendRowChoreo.run();
  
      // return code of zero (0) means success
      if (returnCode == 0) {
        Console.println("Success! Appended " + rowData);
        Console.println("");
      } else {
        // return code of anything other than zero means failure  
        // read and display any error messages
        while (AppendRowChoreo.available()) {
          char c = AppendRowChoreo.read();
          Console.print(c);
        }
      }
  
      AppendRowChoreo.close();
      
      i = 0;
      Ptot = 0;
    }
    else {
      i++;
      Ptot+=Prms;
    }
    time = millis();
  }
}
