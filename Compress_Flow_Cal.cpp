#include<iostream>
#include<cmath>
#include<iomanip>
#define PRCS 4 //note, no semicolon
#define PI 3.142
#define PRSCRIT 0.001 // For nozzle converging (and diverging) programs. Need abs(Pb - crit_Pressure) < PRSCRIT (units MPa) to choose critical pressure condition. If Pb == crit_pressure, need to be same down to 6 d.p --> unrealistic
using namespace std;

void isentropic_cal(double flow_prop[], int size, double GasProp[]);
void shock_cal(double shock_prop[], int size, double GasProp[]);
double shock_angle_cal(double shock_prop[], int size, double gamma);
void expand_fan_cal(double fan_prop[], int size, double GasProp[]);
double expan_fan_v_cal(double M, double gamma, int mode);
void nozzle_con(double nozzle_prop[], int size, double GasProp[]);
void nozzle_con_div(double nozzle_prop[], int size, double GasProp[]);


// Program is to calculate the Stagnation / Total properties of a compressible flow - (Total) Pressure, (Total) Temperature, (Total) Density, Area-Mach Number r/n
// Program is to calculate Normal/Oblique shock r/n
// Program is to calculate whether a converging-diversign/converging nozzle is choked

/*
*******HOW IT WORKS*******
PROGRAM 1 - Isentropic flow r/n calculator: Enter property values (e.g Mach number, pressure, total temperature etc). 
											Value of 0 --> ignore calculation of that flow property and its corresponding total flow property.
											Value of -1 --> To calculate that (total) flow property.
											In each run, program EITHER finds (total) flow property for given Mach Number OR find Mach Number for a given flow property and its corresponding total flow property.
											Program unable to find Mach Number first then find other (total) flow properties using the found Mach Number ==> NEED TO RE-RUN PROGRAM USING FOUND MACH NUMBER to find
											other (total) flow properties.

PROGRAM 2A - Normal / Oblique shock r/n calculator: Enter shock property values (e.g Mach number, pressure, temperature before / after shock)
													Value of 0 --> ignore calculation of that shock property (for pressure, temperature and total pressure, it inclds both BEFORE and AFTER shock if either has value of 0).
													Value of -1 --> To calculate that (total) flow property.
													For normal shock --> enter 90 DEG for beta; enter 0 for theta [IMPT]
													If Shockwave angle unknown, value of 0 in Flow / Wall angle taken as 0 DEG. Field will not be ignored.
													If Shockwave angle known, Flow / Wall angle will be calculated. Any entered Flow / Wall angle value will be overwritten.
													Shockwave angle and Flow / Wall angle values should not exceed 90 DEG.
													In each run, program EITHER finds Mach Number BEFORE shock for given shock conditions OR finds various shock conditions for a given Mach Number BEFORE shock 
													(i.e for given M1).
													Program unable to find M1 using a given shock conditions then find other shock conditions ==> NEED TO RE-RUN PROGRAM USING FOUND M1.

PROGRAM 2B - Expansion fan r/n calculator:  Enter wave properties (e.g Mach number, expansion fan angle). 
											Value of 0 --> ignore calculation of that fan property.
											Value of -1 --> To calculate that expansion fan property.
											In each run, program EITHER finds M1, finds M2 OR finds Flow / Wall angle theta.
											Flow across expansion fan is isentropic, therefore Tt1 = Tt2 & Pt1 = Pt2. To find required flow properties BEFORE or AFTER expansion fan: use Isentropic flow r/n.

PROGRAM 3A - Converging Nozzle choke check:	Enter flow properties in nozzle.
											Value of 0 --> not using that flow property.
											Value of -1 in Pb field --> Only checking for critical back pressure.
											If Total Pressure property not used, Nozzle entrance pressure AND Mach Number MUST be used.
											Flow through nozzle assumed to be entirely isentropic.
											If nozzle total / stagnation pressure unknown, enter Nozzle entrance Pressure + Mach Number 
											OR re-run the program, use Isentropic Calculator mode (Program Function 1) to find total / stagnation pressure.
											If back / ambient pressure unknown, re-run the program, use Isentropic Calculator mode (Program Function 1) to find back / ambient pressure.
											In each run, program finds whether the CONVERGING nozzle is choked for given flow conditions.



PROGRAM 3B - Converging-Diverging Nozzle choke check:	Enter flow properties in nozzle.
														All flow properties must be known. If fields unknown, use Isentropic Calculator to find requried information.
														Value of -1 in Pb field --> Only checking for critical back pressueres.
														Value of > 0 in Pb field --> Find if nozzle choked + critical back pressures + presence of shockwave / expansion fan (and their location) + exit Mach Number, 
														(Total) Temp and (Total) Pressure.

														If no shock is present, flow through nozzle is assumed to be entirely isentropic.
														If shock is present, Total Temperature is constant throughout the nozzle. Isentropic flow assumed before and after shock but not through shock region.

														If underexpanded / overexpanded nozzle --> user will be prompted to rerun program using Shock calculator (Program Mode 2) to calculate exit Mach Number, 
														(Total) Temp and
														(Total) Pressure
*/

// Creator: LAURENCE LU
// Ver 24.08.2022


// NOTE: use pow(val, power) fn to get power

int main() {
	// Step 0: Set output to be fixed and d.p to 4 --> for more info: https://cplusplus.com/reference/ios/fixed/
	cout.precision(PRCS);
	cout << fixed;

	cout << "Compressible flow calculator program. \nWriiten in C++. \nCreator: LAURENCE LU \nVer 24.08.2022 \n\n";

	// Step 1: Select program function. 1 - Stag point r/n  2 - Shock r/n  3 - Nozzle
	int PrgSelect, check_valid_input;
	check_valid_input = 1;
	while (check_valid_input == 1) {
		cout << "Choose prgram function. 1 - Isentropic flow calculator, Total/Stagnation flow properties. 2 - Shockwave/Expansion Fan calculator. 3 - Nozzle choke check. \nProgram function: ";
		cin >> PrgSelect;
		cout << "\n";

		if (PrgSelect == 1 || PrgSelect == 2 || PrgSelect == 3) {
			check_valid_input = 0;
		}
		else {
			cout << "[ERROR] Entered wrong number. Please enter value between 1 to 3 only. \n \n";
		}
	}
	
	
	// Step 2: Enter gas properties
	int GasPropSelect;
	check_valid_input = 1;
	while (check_valid_input == 1) {
		cout << "Choose gas properties. 1 - Cold air (GAMMA 1.4). 2 - Hot air (GAMMA 1.33). 3 - Others, manual input. \nGas properties: ";
		cin >> GasPropSelect;
		cout << "\n";

		if (GasPropSelect == 1 || GasPropSelect == 2 || GasPropSelect == 3) {
			check_valid_input = 0;
		}
		else {
			cout << "[ERROR] Entered wrong number. Please enter value between 1 to 3 only. \n \n";
		}
	}

	double GasProp[3] = { 0 }; //GasProp[0] = Gamma; GasProp[1] = Cp; GasProp[2] = Cv
	switch (GasPropSelect) {
		case 1: // Cold air
			GasProp[0] = 1.4;
			GasProp[1] = 1.004; //kJ/kg.K
			GasProp[2] = GasProp[1] / GasProp[0];
			break;

		case 2: // Hot air
			GasProp[0] = 1.33;
			GasProp[1] = 1.156; //kJ/kg.K
			GasProp[2] = GasProp[1] / GasProp[0];
			break;

		case 3: // Manual Entry
			cout << "Gamma value: ";
			cin >> GasProp[0];

			cout << "Spercific Heat Value constant Pressure Cp [kJ/kg.K]: ";
			cin >> GasProp[1];

			cout << "\n";

			GasProp[2] = GasProp[1] / GasProp[0];
			break;
			
	}	

	// Step 3: Isentropic flow r/n, Total/Stagnation flow properties calcualte
	if (PrgSelect == 1) {
			
		// Get required info
		double total_prop[9] = { 0 }; //NOTE: unable to initialise variable within a SWITCH but able to do it within a IF-ELSE
		/*
		total_prop[0] = Mach No.
		total_prop[1] = Total Temp (K)
		total_prop[2] = Temp (K)
		total_prop[3] = Total Pressure
		total_prop[4] = Pressure
		total_prop[5] = Total Density (kg/m^3)
		total_prop[6] = Density (kg/m^3)
		total_prop[7] = Area (A)
		total_prop[8] = Sonic Area (A*)
		*/

		cout << "***ISENTROPIC FLOW CALCULATOR*** \n \n";

		cout << "Enter isentropic flow properties. \nIf finding the value of stated property / property unknown, enter -1. \nIf not finding a flow property, enter 0. \n";
		cout << "E.g If finding the Total Pressure only, Enter -1 in the Total Pressure field. Enter known Mach Number and Flow Pressure in respective fields. Enter 0 in all other fields. \n\n";
		cout << "In each run, program EITHER finds (total) flow property for given Mach Number OR find Mach Number for a given flow property and its corresponding total flow property.\n";
		cout << "Program is unable to find Mach Number first then find other (total) flow properties using the found Mach Number. \nNEED TO RE-RUN PROGRAM USING FOUND MACH NUMBER TO FIND OTHER (TOTAL) FLOW PROPERTIES. \n\n";

		cout << "Enter Mach Number, M................................................ ";
		cin >> total_prop[0];

		cout << "Enter Total Temperature, Tt (K)..................................... ";
		cin >> total_prop[1];

		cout << "Enter Flow Temperature, T (K)....................................... ";
		cin >> total_prop[2];

		cout << "Enter Total Pressure, Pt (user defined units, ensure consistency)... ";
		cin >> total_prop[3];

		cout << "Enter Flow Pressure, P (user defined units, ensure consistency)..... ";
		cin >> total_prop[4];

		cout << "Enter Total Density, rho_t (kg/m^3)................................. ";
		cin >> total_prop[5];

		cout << "Enter Flow Density, rho (kg/m^3).................................... ";
		cin >> total_prop[6];

		cout << "Enter Nozzle Area, A (m^2).......................................... ";
		cin >> total_prop[7];

		cout << "Enter Nozzle Throat / Sonic Area, A* (m^2).......................... ";
		cin >> total_prop[8];

		cout << "\n";

		// Send info to isentropic r/n sub-function
		isentropic_cal(total_prop, 9, GasProp);
	}
	else if (PrgSelect == 2) {
		// Step 4: Shock/Expansion fan r/n calculate

		cout << "***SHOCKWAVE / EXPANSION FAN CALCULATOR*** \n \n";
		// Choose sub-prg
		int shock_prog = -1;
		while (shock_prog == -1) {
			cout << "Choose type of shock. 1 - Normal/Oblique Shockwave. 2 - Expansion wave. \nShock type: ";
			cin >> shock_prog;
			cout << "\n";

			if (shock_prog == 1 || shock_prog == 2) {
				// do nothing
			}
			else {
				shock_prog = -1;
				cout << "[ERROR] Entered wrong number. Please enter value between 1 to 2 only. \n\n";
			}
		}

		// Get required info
		double shock_prop[12] = { 0 };
		switch (shock_prog) {
			case 1: // Normal/Oblique Shockwave
				cout << "***NORMAL / OBLIQUE SHOCKWAVE MODE***\n\n";
				cout << "Enter wave properties. \nIf finding the value of stated property / property unknown, enter -1. \nIf not finding a flow property, enter 0. \n";
				cout << "E.g If finding Pressure 1 only, Enter -1 in the Pressure 1 field. Enter known Mach Number M1 and Pressure 2 in respective fields. Enter 0 in all other fields. \n \n";
				cout << "If Shockwave angle unknown, value of 0 in Flow / Wall angle taken as 0 DEG. Field will not be ignored.\n";
				cout << "If Shockwave angle known, Flow / Wall angle will be calculated. Any entered Flow / Wall angle value will be overwritten.\n";
				cout << "Shockwave angle and Flow / Wall angle values should not exceed 90 DEG. \n\n";
				cout << "Subscript 1 denotes flow BEFORE wave. Subscript 2 denotes flow AFTER wave. \n\n";
				
				cout << "In each run, program EITHER finds Mach Number BEFORE shock for given shock conditions OR finds various shock conditions for a given Mach Number BEFORE shock (i.e M1).\n";
				cout << "Program unable to find M1 first then find other shock conditions. \nNEED TO RE-RUN PROGRAM USING FOUND M1 TO FIND OTHER SHOCK CONDITIONS.\n\n";
				/*
				shock_prop[0] = M1
				shock_prop[1] = M2
				shock_prop[2] = beta
				shock_prop[3] = theta
				shock_prop[4] = T1
				shock_prop[5] = T2
				shock_prop[6] = Tt1
				shock_prop[7] = Tt2
				shock_prop[8] = P1
				shock_prop[9] = P2
				shock_prop[10] = Pt1
				shock_prop[11] = Pt2
				*/
				cout << "Enter Mach Number 1, M1.................................................. ";
				cin >> shock_prop[0];

				cout << "Enter Mach Number 2, M2.................................................. ";
				cin >> shock_prop[1];

				cout << "Enter Shockwave angle, beta (DEG). If Normal Shock, enter 90............. ";
				cin >> shock_prop[2];

				cout << "Enter Flow / Wall angle, theta (DEG). If Normal Shock, enter 0........... ";
				cin >> shock_prop[3];

				cout << "Enter Temperature 1, T1 (K).............................................. ";
				cin >> shock_prop[4];

				cout << "Enter Temperature 2, T2 (K).............................................. ";
				cin >> shock_prop[5];

				cout << "Enter Total Temperature 1, Tt1 (K)....................................... ";
				cin >> shock_prop[6];

				cout << "Enter Total Temperature 2, Tt2 (K)....................................... " << shock_prop[6] << "\n";
				shock_prop[7] = shock_prop[6];

				cout << "Enter Pressure 1, P1 (user defined units, ensure consistency)............ ";
				cin >> shock_prop[8];

				cout << "Enter Pressure 2, P2 (user defined units, ensure consistency)............ ";
				cin >> shock_prop[9];

				cout << "Enter Total Pressure 1, Pt1 (user defined units, ensure consistency)..... ";
				cin >> shock_prop[10];

				cout << "Enter Total Pressure 2, Pt2 (user defined units, ensure consistency)..... ";
				cin >> shock_prop[11];

				break;

			case 2: // Expansion fan
				cout << "***EXPANSION FAN MODE***\n\n";
				cout << "Enter wave properties. \nIf finding the value of stated property / property unknown, enter -1. \nIf not finding a flow property, enter 0. \n";
				cout << "E.g If Mach Number M1, Enter -1 in the Mach Number 1 field. Enter known Mach Number 2 and Expansion Fan angle theta. Enter 0 in all other fields. \n \n";
				cout << "Subscript 1 denotes flow BEFORE wave. Subscript 2 denotes flow AFTER wave. \n\n";
				cout << "In each run, program EITHER finds M1, finds M2 OR finds Flow / Wall angle theta.\n\n";
				cout << "Flow across expansion fan is isentropic, therefore Tt1 = Tt2 & Pt1 = Pt2. \nTo find required flow properties BEFORE or AFTER expansion fan: \n";
				cout << "Use Isentropic Calculator mode (Program Function 1) --> enter Mach Number found + any other corresponding flow property values.\n";
				cout << "E.g To find Pressure AFTER expansion wave (P2): \nFind M2 using known M1 and theta \n--> Switch to Program Function 1 \n--> Enter value of M2 in Mach Number field and Pt2 in Total Pressure field \n--> Enter -1 in Pressure field \n--> Pressure value calculated is P2. \n\n";
				/*shock_prop[0] = M1
					shock_prop[1] = M2
					shock_prop[2] = theta	
				*/
				cout << "Enter Mach Number 1, M1.................................................. ";
				cin >> shock_prop[0];

				cout << "Enter Mach Number 2, M2.................................................. ";
				cin >> shock_prop[1];

				cout << "Enter Flow / Wall angle, theta (DEG)..................................... ";
				cin >> shock_prop[2];

				break;
		}
		cout << "\n";

		// Send to sub-function to calculate
		switch (shock_prog) {
			case 1: // Normal / Oblique shock calculate
				shock_cal(shock_prop, 12, GasProp);
				break;

			case 2: // Expansion fan calculate
				expand_fan_cal(shock_prop, 3, GasProp);
				break;
		}


	}
	else if (PrgSelect == 3) {
		// Step 5: Nozzle chock check calculate

		cout << "***NOZZLE CHOKE CHECK*** \n \n";

		// Choose sub-prg
		int nozzle_prog = -1;
		while (nozzle_prog == -1) {
			cout << "Choose type of nozzle. 1 - Converging nozzle. 2 - Converging-Diverging nozzle. \nNozzle type: ";
			cin >> nozzle_prog;
			cout << "\n";

			if (nozzle_prog == 1 || nozzle_prog == 2) {
				// do nothing
			}
			else {
				nozzle_prog = -1;
				cout << "[ERROR] Entered wrong number. Please enter value between 1 to 2 only. \n\n";
			}
		}
		

		// Get required info
		double nozzle_prop[5] = { 0 }; 

		switch (nozzle_prog) {
			case 1: // Converging nozzle
				cout << "***CONVERGING NOZZLE MODE***\n\n";
				cout << "Enter flow properties in nozzle. \nIf not using a flow property, enter 0.\n";
				cout << "If checking for critical back / ambient pressure (i.e the point where nozzle chokes), \nenter -1 in back / ambient pressure field.\n\n";
				
				cout << "In each run, program finds whether the CONVERGING nozzle is choked for given flow conditions.\n";
				cout << "Flow through nozzle assumed to be entirely isentropic. \n\n";
				cout << "If nozzle total / stagnation pressure unknown, \n";
				cout << "Enter Nozzle entrance Pressure + Mach Number OR re-run the program, use Isentropic Calculator mode (Program Function 1) to find total / stagnation pressure. \n\n";
				cout << "To find other flow properties at entrance / exit of nozzle, use Isentropic Calculator mode (Program Function 1) and respective known flow properties. \n";
				cout << "E.g To find Mach Number after Nozzle Exit / at Nozzle Exit if nozzle not choked, \n";
				cout << "re-run the program using Isentropic Calculator mode (Program Function 1) and enter Nozzle Total Pressure and Back Pressure. \n";
				cout << "Mach number at Nozzle Exit is 1 if nozzle choked.\n\n";

				cout << "If back / ambient pressure unknown, re-run the program, use Isentropic Calculator mode (Program Function 1) to find back / ambient pressure. \n\n";

				/*
				nozzle_prop[0] = Pt
				nozzle_prop[1] = P(in)
				nozzle_prop[2] = M(in)
				nozzle_prop[3] = P(b)
				
				*/
				cout << "Nozzle Total / Stagnation Pressure, Pt (MPa)................. ";
				cin >> nozzle_prop[0];

				cout << "Nozzle entrance Pressure, P (MPa)............................ ";
				cin >> nozzle_prop[1];

				cout << "Nozzle entrance Mach Number, M............................... ";
				cin >> nozzle_prop[2];

				cout << "Back pressure / Ambient pressure, Pb (MPa)................... ";
				cin >> nozzle_prop[3];

				break;
			case 2: // Converging - diverging nozzle
				cout << "***CONVERGING-DIVERGING NOZZLE MODE***";
				cout << "Enter flow properties in nozzle. \n";
				cout << "If checking for critical back / ambient pressures only, enter -1 in back / ambient pressure field. \n";
				//cout << "Nozzle exit flow angle must be between 0 DEG to 90 DEG. \n";
				cout << "All fields must be known. If certain fields are unknown, consider using Isentropic Calculator mode (Program Function 1) to find required information.\n\n";
				//cout << "If nozzle exit flow angle field unknown, enter 0. \n\n";

				cout << "In each run, \nIf back / ambient pressure unknown, program finds all critical back / ambient pressures only.\n";
				cout << "If back / ambient pressure known, program finds whether the CONVERGING-DIVERGING nozzle is choked for given flow conditions and all critical back / ambient pressures.\n";
				cout << "Thereafter, it finds if a Shockwave / Expansion fan is present, location of Shock and Mach Number, (Total) Temperature & (Total) Pressure at Nozzle Exit.\n\n";

				cout << "If no shock is present, flow through nozzle is assumed to be entirely isentropic. \n";
				cout << "If shock is present, Total Temperature is constant throughout the nozzle. Isentropic flow assumed before and after shock but not through shock region. \n\n";

				/*
				nozzle_prop[0] = Pt_in
				nozzle_prop[1] = Tt_in
				nozzle_prop[2] = At
				nozzle_prop[3] = Ae
				nozzle_prop[4] = Pb
				*/

				cout << "Nozzle entrance Total / Stagnation Pressure, Pt_in (MPa)..... ";
				cin >> nozzle_prop[0];

				cout << "Nozzle entrance Total / Stagnation Pressure, Tt_in (K)....... ";
				cin >> nozzle_prop[1];

				cout << "Nozzle Throat Area, At (m^2)................................. ";
				cin >> nozzle_prop[2];

				cout << "Nozzle Exit Area, Ae (m^2)................................... ";
				cin >> nozzle_prop[3];

				cout << "Nozzle exit Back / Ambient Pressure, Pb (MPa)................ ";
				cin >> nozzle_prop[4];

				//cout << "Nozzle exit flow angle, theta (DEG). Enter 0 if unknown........................................ ";
				//cin >> nozzle_prop[5];

				break;
		}

		cout << "\n";


		// Sent to sub-function to calculate
		switch (nozzle_prog) {
			case 1: // Converging nozzle
				nozzle_con(nozzle_prop, 4, GasProp);
				break;

			case 2: // Converging - diverging nozzle
				nozzle_con_div(nozzle_prop, 5, GasProp);
				break;
		}
	}
	
	// Step 6: Hold the program, prevent console window from closing until user is done
	
	cout << "\n";
	char to_end;
	int check_end = 0;

	while (check_end == 0) {
		cout << "To end program, press E: ";
		cin >> to_end;
		cout << "\n";

		if (to_end == 'e' || to_end == 'E') {
			check_end = 1;
			break;
		}
		else {
			// check_end remains 0
		}
	}
	
	return 0;
}



// SUB-FUNCTION is to calculate isentropic flow properties
void isentropic_cal(double flow_prop[], int size, double GasProp[]) {
	/*
	flow_prop[0] = Mach No.
	flow_prop[1] = Total Temp (K)
	flow_prop[2] = Temp (K)
	flow_prop[3] = Total Pressure
	flow_prop[4] = Pressure
	flow_prop[5] = Total Density (kg/m^3)
	flow_prop[6] = Density (kg/m^3)
	flow_prop[7] = Area (A)
	flow_prop[8] = Sonic Area (A*)
	If value 0, ignore. If value -1, find that property

	GasProp[0] = Gamma
	GasProp[1] = Cp
	GasProp[2] = Cv
	*/

	double ratio, gamma;
	gamma = GasProp[0];

	int i;

	if (flow_prop[0] > 0) { // Mach Number known, finding other flow properties

		for (i = 1; i <= 5; i += 2) { // Find Temperature, Pressure and Density

			if (flow_prop[i] != 0 && flow_prop[i + 1] != 0) { // Find flow cond / Total cond
				
				ratio = 1 + 0.5 * (gamma - 1) * flow_prop[0] * flow_prop[0]; // Temperature ratio
				if (i == 3) {
					ratio = pow(ratio, (gamma / (gamma - 1))); // Pressre ratio
				}
				else if (i == 5) {
					ratio = pow(ratio, (1 / (gamma - 1))); // Density ratio
				}

				if (flow_prop[i] < 0 && flow_prop[i + 1] > 0) { // Cond known, find Total Cond
					flow_prop[i] = ratio * flow_prop[i + 1];
				}
				else if (flow_prop[i] > 0 && flow_prop[i + 1] < 0) { // Total Cond known, find Cond
					flow_prop[i + 1] = (1 / ratio) * flow_prop[i];
				}
				else {
					switch (i) {
						case 1:
							cout << "[ERROR] Input for TEMPERATURE RATIO improperly defined!\n \n";
							break;
						case 3:
							cout << "[ERROR] Input for PRESSURE RATIO improperly defined!\n \n";
							break;
						case 5:
							cout << "[ERROR] Input for DENSITY RATIO improperly defined!\n \n";
							break;
					}
					if (flow_prop[i] < 0 && flow_prop[i+1] < 0) {
						flow_prop[i] = 0;
						flow_prop[i + 1] = 0;
					}
				}
			}
			else { // One of the variables 0, ignore
				flow_prop[i] = 0;
				flow_prop[i + 1] = 0;
			}
		}

		// Find Area-Mach Number r/n
		if (flow_prop[7] != 0 && flow_prop[8] != 0) {
			ratio = (1 / (flow_prop[0] * flow_prop[0])) * pow((2 / (gamma + 1)) * (1 + 0.5 * (gamma - 1) * flow_prop[0] * flow_prop[0]), ((gamma + 1) / (gamma - 1)));
			ratio = sqrt(ratio);

			if (flow_prop[7] < 0 && flow_prop[8] > 0) { // Area unknown, A* known
				flow_prop[7] = ratio * flow_prop[8];
			}
			else if (flow_prop[7] > 0 && flow_prop[8] < 0) { // Area known, A* unknown
				flow_prop[8] = (1 / ratio) * flow_prop[7];
			}
			else {
				cout << "[ERROR] Input for AREA-MACH NUMBER RATIO improperly defined!\n \n";
				if (flow_prop[7] < 0 && flow_prop[8] < 0) {
					flow_prop[7] = 0;
					flow_prop[8] = 0;
				}
			}
		}
		else { // One of the variables 0, ignore
			flow_prop[7] = 0;
			flow_prop[8] = 0;
		}
	}
	else if (flow_prop[0] < 0) { // Other flow properties known, finding Mach Number
		// Check which property ratio to use, if two or more entered, require user selection
		int check[4] = { 0 }; // [0]: Temp; [1]: Pressure; [2]: Density; [3]: Area
		int flowIdx = -1;
		for (i = 1; i < size; i += 2) {
			if (flow_prop[i] > 0 && flow_prop[i + 1] > 0) {
				check[i - (i/2 +1)] = 1;
			}
			else {
				flow_prop[i] = 0;
				flow_prop[i + 1] = 0;
			}
		}
		for (i = 0; i < 4; i++) { // flowIdx == -1 means not initialised. If flowIdx == -2, conflict, multiple property ratios defined.
			if (check[i] == 1 && flowIdx == -1) {
				flowIdx = i;
			}
			else if (check[i] == 1 && flowIdx != -1) {
				flowIdx = -2;
				while (flowIdx == -2) {
					cout << "Multiple flow properties detected! Please indicate which isentropic total flow ratio to use to calculate Mach Number. 1 - Temperature ratio. 2 - Pressure ratio. 3 - Density ratio. 4 - Area-Mach Number relation. \nFlow ratio: ";
					cin >> flowIdx;
					cout << "\n";
					if (flowIdx == 1 || flowIdx == 2 || flowIdx == 3 || flowIdx == 4) {
						// Need to handle for user to choose properly defined properties. This user override doesn't take into account if user choose underdefined or -ignore- property
						if (flow_prop[flowIdx + flowIdx - 1] <= 0 || flow_prop[flowIdx + flowIdx] <= 0) {
							cout << "[ERROR] Flow property choosen improperly defined! Choose other flow property! \n \n";
							flowIdx = -2;
						}
					}
					else {
						flowIdx = -2;
						cout << "[ERROR] Entered wrong number. Please enter value between 1 to 4 only. \n \n";
					}
				}
				flowIdx = flowIdx--;
				break;
			}
		}
		if (flowIdx == -1) { // All flow properties 0, unable to proceed
			cout << "[ERROR] Input underdefined! \n \n";
			return;
		}

		// Calculate Mach Number
		int AreaMachType = 0;
		int itr;
		double MachItr1, MachItr2, crit;

		switch (flowIdx) {
			case 0: // Temperature ratio
				ratio = flow_prop[1] / flow_prop[2];
				flow_prop[0] = sqrt((ratio - 1) * 2 / (gamma - 1));
				break;

			case 1: // Pressure ratio
				ratio = flow_prop[3] / flow_prop[4];
				flow_prop[0] = sqrt((pow(ratio, (gamma - 1) / gamma) - 1) * 2 / (gamma - 1));
				break;

			case 2: // Density ratio
				ratio = flow_prop[5] / flow_prop[6];
				flow_prop[0] = sqrt((pow(ratio, (gamma - 1)) - 1) * 2 / (gamma - 1));
				break;

			case 3: // Area-Mach Number r/n. To note: https://www.grc.nasa.gov/www/winddocs/utilities/b4wind_guide/mach.html
				
				if ((flow_prop[7] / flow_prop[8]) == 1) { // Sonic condition
					cout << "Area ratio is 1, sonic condition. \n \n";
					flow_prop[0] = 1;
					break;
				}
				
				while (AreaMachType == 0) {
					cout << "Choose Mach Number solution type. 1 - SUPERSONIC Mach Number solution. 2 - SUBSONIC Mach Number solution. \nMach number solution: ";
					cin >> AreaMachType;
					cout << "\n";
					if (AreaMachType == 1) {
						MachItr1 = 5.001; // Initial guess Supersonic
					}
					else if (AreaMachType == 2) {
						MachItr1 = 0.001; // Initial guess Subsonic
					}
					else {
						AreaMachType = 0;
						cout << "[ERROR] Entered wrong number. Pease enter value between 1 to 2 only. \n \n";
					}
				}
				cout << "Enter convergence criterion. Default value: 0.0001. A lower number would yield more accurate results. \nConvergence criterion: ";
				cin >> crit;
				cout << "\n";

				ratio = flow_prop[7] / flow_prop[8];

				for (itr = 1; itr <= 100; itr++) { // Solving using the Newton Rhapson method. x(n+1) = x(n) - f(x(n))/f'(x(n)). See ME5309 scrap workings in iPad for more info.
					
					MachItr2 = MachItr1 - (0.5 * (gamma + 1) * pow(ratio, 2 * (gamma - 1) / (gamma + 1)) * pow(MachItr1, (gamma - 1) / (gamma + 1)) - 0.5 * (gamma - 1) * MachItr1 - 1) / (0.5 * (gamma - 1) * (pow(ratio, 2 * (gamma - 1) / (gamma + 1)) * pow(MachItr1, -2 / (gamma + 1)) - 1));
					cout << "Iteration: " << itr << ". Convergence: " << abs(MachItr2 - MachItr1) << ".\n";
					// cout << "MachItr1: " << MachItr1 << " MachItr2: " << MachItr2 << "\n\n"; FOR DEBUGGING

					if (abs(MachItr2 - MachItr1) < crit) {
						cout << "Iteration completed at iteration " << itr << " with convergence " << abs(MachItr2 - MachItr1) << ".\n\n";
						break;
					}

					if (itr == 100) {
						cout << "[ERROR] Maxed out iteration. Adjust convergence criterion, initial guess or iteration number. FOR DEBUGGING. PROGRAM EXIT. \n\n";
						return;
					}
					MachItr1 = MachItr2;
				}
				flow_prop[0] = sqrt(MachItr2);

				break;
		}

	}
	else { // Entered wrong value / Entered 0 --> not finding Mach Number --> Error
		cout << "[ERROR] Input for MACH NUMBER underdefined! Cannot be zero! \n \n";
		return;
	}

	// Output solution to console
	cout << "***ISENTROPIC FLOW CALCULATOR SOLUTION***\n\n";
	cout << "If value is 0, calculation on that flow property not performed.\n";
	cout << "Mach Number (M)........................ " << flow_prop[0] << " \n";
	cout << "Total Temperature (Tt)................. " << flow_prop[1] << " K\n";
	cout << "Temperature (T)........................ " << flow_prop[2] << " K\n";
	cout << "Total Pressure (Pt).................... " << flow_prop[3] << " (user defined units)\n";
	cout << "Pressure (P)........................... " << flow_prop[4] << " (user defined units)\n";
	cout << "Total Density (rho_t).................. " << flow_prop[5] << " kg/m^3\n";
	cout << "Density (rho).......................... " << flow_prop[6] << " kg/m^3\n";
	cout << "Area (A)............................... " << flow_prop[7] << " m^2\n";
	cout << "Throat / Sonic Area (A*)............... " << flow_prop[8] << " m^2\n";

	return;
}



// SUB-FUNCTION is to calculate flow properties across normal / oblique shock
void shock_cal(double shock_prop[], int size, double GasProp[]) {
	/*
	shock_prop[0] = M1
	shock_prop[1] = M2
	shock_prop[2] = beta
	shock_prop[3] = theta
	shock_prop[4] = T1
	shock_prop[5] = T2
	shock_prop[6] = Tt1
	shock_prop[7] = Tt2
	shock_prop[8] = P1
	shock_prop[9] = P2
	shock_prop[10] = Pt1
	shock_prop[11] = Pt2
	*/
	
	// Step 0: Check whether shock angles exceed 90 DEG or not properly defined. Check whether M1 > 0.
	if (shock_prop[0] <= 1 && shock_prop[0] >= 0) {
		cout << "[ERROR] Mach Number 1 (M1) cannot be less than 1! Shockwave unable to form in subsonic conditions! Ending program! \n\n";
		return;
	}
	if (shock_prop[2] > 90 || shock_prop[3] > 90) {
		cout << "[ERROR] Shock angle and/or Flow / Wall angle cannot exceed 90 DEG! Ending program! \n\n";
		return;
	}
	if (shock_prop[2] < 0 && shock_prop[3] < 0) {
		cout << "[ERROR] Shock angle and Flow / Wall angle cannot be both unknown at the same time! Ending program! \n\n";
		return;
	}
	if (shock_prop[2] == 0) {
		cout << "[ERROR] Shock angle cannot be zero! If normal shock, enter 90 DEG! Ending program! \n\n";
		return;
	}

	double M1, gamma;
	gamma = GasProp[0];
	

	if (shock_prop[0] > 0) {
		// Step 1: Get beta / theta if need be --> Get M1
		M1 = shock_angle_cal(shock_prop, 12, gamma);
		if (M1 < 0) {
			return;
		}

		// Step 2: Calculate shock property values with known M1. If entered M1 <= 1 --> error already captured in step 0.
		double ratio = 0;
		
		// Find M2
		if (shock_prop[1] < 0) {
			shock_prop[1] = (1 + 0.5 * (gamma - 1) * M1 * M1) / (gamma * M1 * M1 - 0.5*(gamma - 1));
			shock_prop[1] = sqrt(shock_prop[1]);
			shock_prop[1] = shock_prop[1] / sin((shock_prop[2] - shock_prop[3]) * PI / 180);
		}
		// Find P2/P1
		if (shock_prop[8] < 0 || shock_prop[9] < 0) {
			ratio = 1 + 2 * gamma * (M1 * M1 - 1) / (gamma + 1);
			if (shock_prop[8] < 0 && shock_prop[9] > 0) {
				shock_prop[8] = (1 / ratio) * shock_prop[9]; // Find P1
			}
			else if (shock_prop[8] > 0 && shock_prop[9] < 0) {
				shock_prop[9] = ratio * shock_prop[8]; // Find P2
			}
			else {
				cout << "[WARNING] Both P1 & P2 unkown! Values taken as 0. \n\n";
				shock_prop[8] = 0;
				shock_prop[9] = 0;
			}
		}
		else if (shock_prop[8] == 0 || shock_prop[9] == 0) {
			shock_prop[8] = 0;
			shock_prop[9] = 0;
		}
		// Find T2/T1
		if (shock_prop[4] < 0 || shock_prop[5] < 0) {
			ratio = 1 + 2 * (gamma - 1) * (gamma * pow(M1, 2) + 1) * (pow(M1, 2) - 1) / pow(M1 * (gamma + 1), 2);
			if (shock_prop[4] < 0 && shock_prop[5] > 0) {
				shock_prop[4] = (1 / ratio) * shock_prop[5]; // Find T1
			}
			else if (shock_prop[4] > 0 && shock_prop[5] < 0) {
				shock_prop[5] = ratio * shock_prop[4]; // Find T2
			}
			else {
				cout << "[WARNING] Both T1 & T2 unknown! Values taken as 0. \n\n";
				shock_prop[4] = 0;
				shock_prop[5] = 0;
			}
		}
		else if (shock_prop[4] == 0 || shock_prop[5] == 0) {
			shock_prop[4] = 0;
			shock_prop[5] = 0;
		}
		// Find Pt2/Pt1
		if (shock_prop[10] < 0 || shock_prop[11] < 0) {
			ratio = pow(((gamma + 1) * M1 * M1) / (2 + (gamma - 1) * M1 * M1), gamma / (gamma - 1)) * pow((gamma + 1) / (2 * gamma * M1 * M1 - gamma + 1), 1 / (gamma - 1));
			if (shock_prop[10] < 0 && shock_prop[11] > 0) {
				shock_prop[10] = (1 / ratio) * shock_prop[11]; // Find Pt1
			}
			else if (shock_prop[10] > 0 && shock_prop[11] < 0) {
				shock_prop[11] = ratio * shock_prop[10]; // Find Pt2
			}
			else {
				cout << "[WARNING] Both Pt1 & Pt2 unknown! Values taken as 0. \n\n";
				shock_prop[10] = 0;
				shock_prop[11] = 0;
			}
		}
		else if (shock_prop[10] == 0 || shock_prop[11]) {
			shock_prop[10] = 0;
			shock_prop[11] = 0;
		}
	}
	else if (shock_prop[0] < 0) {
		// Step 3: Calculate M1
		int shockIdx = -1; // shockIdx = -1 means not initialised / issues with the property pair
		double ratio = 0;
		
		if (shock_prop[2] <= 0) {
			cout << "[ERROR] Shockwave angle needs to be known / cannot be zero! If Normal Shock, enter 90 DEG. Ending program! \n\n";
			return;
		}

		while (shockIdx == -1) {
			cout << "Select which shock flow properties to calculate Mach Number 1 (M1). 1 - Mach Number 2 (M2). 2 - Temperature ratio (T2/T1). 3 - Pressure ratio (P2/P1).\nShock flow property: ";
			cin >> shockIdx;
			cout << "\n";

			if (shockIdx == 1) {
				if (shock_prop[1] <= 0) {
					shockIdx = -2;
					cout << "[ERROR] Mach Number 2 is not fully defined! \n\n";
					while (shockIdx == -2) {
						cout << "Choose other shock flow properties or terminate program? \n0 - Terminate program. 1 - Choose other shock flow properties. Selection: ";
						cin >> shockIdx;
						cout << "\n";

						if (shockIdx == 0) {
							cout << "Terminating program! \n\n";
							return;
						}
						else if (shockIdx == 1) {
							shockIdx = -1;
						}
						else {
							cout << "[ERROR] Entered wrong number. Please enter value 0 or 1 only. \n\n";
							shockIdx = -2;
						}
					}
				}
			}
			else if (shockIdx == 2) {
				if (shock_prop[4] <= 0 || shock_prop[5] <= 0) {
					shockIdx = -2;
					cout << "[ERROR] Temperature 1 and/or 2 are not fully defined!\n\n";
				}
				if (shockIdx == -2) {
					while (shockIdx == -2) {
						cout << "Choose other shock flow properties or terminate program? \n0 - Terminate program. 1 - Choose other shock flow properties. Selection: ";
						cin >> shockIdx;
						cout << "\n";

						if (shockIdx == 0) {
							cout << "Terminating program! \n\n";
							return;
						}
						else if (shockIdx == 1) {
							shockIdx = -1;
						}
						else {
							cout << "[ERROR] Entered wrong number. Please enter value 0 or 1 only. \n\n";
							shockIdx = -2;
						}
					}
				}
			}
			else if (shockIdx == 3) {
				if (shock_prop[8] <= 0 || shock_prop[9] <= 0) {
					shockIdx = -2;
					cout << "[ERROR] Pressure 1 and/or 2 are not fully defined!\n\n";
				}
				if (shockIdx == -2) {
					while (shockIdx == -2) {
						cout << "Choose other shock flow properties or terminate program? \n0 - Terminate program. 1 - Choose other shock flow properties. Selection: ";
						cin >> shockIdx;
						cout << "\n";

						if (shockIdx == 0) {
							cout << "Terminating program! \n\n";
							return;
						}
						else if (shockIdx == 1) {
							shockIdx = -1;
						}
						else {
							cout << "[ERROR] Entered wrong number. Please enter value 0 or 1 only. \n\n";
							shockIdx = -2;
						}
					}
				}
			}
			else {
				cout << "[ERROR] Entered wrong number. Please enter value between 1 to 3 only. \n\n";
				shockIdx = -1;
			}
		}

		if (shockIdx == 1) { // Find M1 using M2
			if (shock_prop[3] < 0) {
				cout << "[ERROR] Flow / Wall angle needs to be known to use this method! Ending program! \n\n";
				return;
			}

			double M2;
			
			M2 = shock_prop[1] * sin((shock_prop[2] - shock_prop[3]) * PI / 180);
			
			M1 = (0.5 * (gamma - 1) * M2 * M2 + 1) / (M2 * M2 * gamma - 0.5 * (gamma - 1));
			M1 = sqrt(M1);
		}
		else if (shockIdx == 2) { // Find M1 using T2/T1
			double a, b, c, soln1, soln2;
			
			ratio = shock_prop[5] / shock_prop[4];

			a = gamma;
			b = 1 - gamma - (ratio - 1) * pow(gamma + 1, 2) / (2 * gamma - 2);
			c = -1;

			soln1 = (-1 * b + sqrt(b * b - 4 * a * c)) / (2 * a);
			soln2 = (-1 * b - sqrt(b * b - 4 * a * c)) / (2 * a);

			cout << "soln1: " << soln1 << "\n";
			cout << "soln2: " << soln2 << "\n";


			if (soln1 > 0 && soln2 < 0) {
				M1 = sqrt(soln1);
			}
			else if (soln1 < 0 && soln2 > 0) {
				M1 = sqrt(soln2);
			}
			else {
				cout << "[ERROR] No real solution for M1 with given values of T1 and T2. Ending program! \n\n";
			}
		}
		else if (shockIdx == 3) { // Find M1 using P2/P1
			ratio = shock_prop[9] / shock_prop[8];

			M1 = ((gamma + 1) / (2 * gamma)) * (ratio - 1) + 1;
			M1 = sqrt(M1);
		}
		shock_prop[0] = M1 / sin(shock_prop[2] * PI / 180);

	} 

	// Step 4: Print results to console
	cout << "***NORMAL / OBLIQUE SHOCK CALCULATOR SOLUTION***\n\n";
	cout << "If value is 0, calculation on that flow property not performed.\n";
	cout << "Mach Number BEFORE shock (M1)........................... " << shock_prop[0] << "\n";
	cout << "Mach Number AFTER shock (M2)............................ " << shock_prop[1] << "\n";
	cout << "Shockwave angle (beta).................................. " << shock_prop[2] << " DEG\n";
	cout << "Flow / Turn / Wall angle (theta)........................ " << shock_prop[3] << " DEG\n";
	cout << "Temperature BEFORE shock (T1)........................... " << shock_prop[4] << " K\n";
	cout << "Temperature AFTER shock (T2)............................ " << shock_prop[5] << " K\n";
	cout << "Total Temperature BEFORE shock (Tt1).................... " << shock_prop[6] << " K\n";
	cout << "Total Temperature AFTER shock (Tt2)..................... " << shock_prop[7] << " K\n";
	cout << "Presure BEFORE shock (P1)............................... " << shock_prop[8] << " (user defined units)\n";
	cout << "Presure AFTER shock (P2)................................ " << shock_prop[9] << " (user defined units)\n";
	cout << "Total Pressure BEFORE shock (Pt1)....................... " << shock_prop[10] << " (user defined units)\n";
	cout << "Totla Pressure AFTER shock (Pt2)........................ " << shock_prop[11] << " (user defined units)\n";

	return;
}



// SUB-FUNCTION is to calculate beta or theta of shockwave. Used only if finding other shock flow properties. Returns M1,n = M1*sin(beta) value --> denoted as M1
double shock_angle_cal(double shock_prop[], int size, double gamma) {
	/*
	flow_prop[0] = Mach No.
	flow_prop[1] = Total Temp (K)
	flow_prop[2] = Temp (K)
	flow_prop[3] = Total Pressure
	flow_prop[4] = Pressure
	flow_prop[5] = Total Density (kg/m^3)
	flow_prop[6] = Density (kg/m^3)
	flow_prop[7] = Area (A)
	flow_prop[8] = Sonic Area (A*)
	If value 0, ignore. If value -1, find that property
	*/

	/*
	KNOWN BUG --> there'll be instances of oscilating or weak shock values being returned even if strong shock option selected if theta is close to theta_max
	*/
	
	double M1;

	if (shock_prop[2] == 90) { // Normal shock
		M1 = shock_prop[0];
		shock_prop[3] = 0; // In case user enter wrong theta value for normal shock case
	}
	else if (shock_prop[2] < 0) { // Oblique shock, beta unknown, theta needs to be known, M1 needs to be known
		if (shock_prop[3] < 0 || shock_prop[0] <= 0) { // Redundency error handling
			cout << "[ERROR] Oblique shock underdefined! Need to know at least one angle! Need to know Mach Number 1 (M1)! M1 cannot be zero! Ending program! \n\n";
			return -1;
		}

		// Solving for beta using theta and M1
		int itr, shock_type;
		double crit, beta1, beta2, theta, fn_beta1, fn_prime_beta1, theta_max;
		M1 = shock_prop[0]; // For ease in the Newton Rhapson method.
		theta = shock_prop[3] * PI / 180; // Cnvert to RAD as trigo fn in C++ only in RAD
		theta_max = (4 / (3 * sqrt(3) * (gamma + 1))) * (pow(M1 * M1 - 1, 1.5) / pow(M1, 2)) * (180 / PI) - 1; // if theta > theta_max --> detached shock. ***VALUE IN DEG*** -1 incld to give calculation leeway

		if (shock_prop[3] >= theta_max) { //theta_max cal in DEG
			cout << "DETACHED SHOCK condition. Unable to proceed. Ending program! \n\n";
			return -1;
		}

		shock_type = 0;
		while (shock_type == 0) {
			cout << "Choose oblique shock type. 1 - STRONG shock solution. 2 - WEAK shock solution (common). \nShock solution: ";
			cin >> shock_type;
			cout << "\n";
			if (shock_type == 1) {
				beta1 = 90 * PI / 180; // Initial guess stong shock. In RAD as trigo fn in C++ in RAD. Value in eqn here in DEG. For theta values close to theta_max --> giving weak shock values
			}
			else if (shock_type == 2) {
				beta1 = 5 * PI / 180; // Initial guess weak shock. In RAD as trigo fn in C++ in RAD. Value in eqn here in DEG.
			}
			else {
				shock_type = 0;
				cout << "[ERROR] Entered wrong number. PLeaser enter value between 1 to 2 only. \n \n";
			}
		}

		cout << "Enter convergence criterion. Default value: 0.0001. A lower number would yield more accurate results. \nConvergence criterion: ";
		cin >> crit;
		cout << "\n";

		for (itr = 1; itr <= 100; itr++) { // Solving using the Newton Rhapson method. x(n+1) = x(n) - f(x(n))/f'(x(n)). See ME5309 scrap workings in iPad for more info.

			if (theta == 0 && shock_type == 1) { // Normal shock
				beta2 = 90 * PI / 180;
				cout << "NO ITERATION. Normal shock conditions.\n\n";
				break;
			}
			if (theta == 0 && shock_type == 2) {
				beta2 = asin(sqrt(1 / pow(M1, 2)));
				cout << "NO ITERATION. Weak shock, no flow / wall angle conditions. \n\n";
				break;
			}

			fn_beta1 = (2 / tan(beta1)) * ((pow(M1 * sin(beta1), 2) - 1) / (M1 * M1 * (gamma + cos(2 * beta1)) + 2)) - tan(theta);

			fn_prime_beta1 = -2 * (pow(M1 * sin(beta1), 2) - 1) / (pow(sin(beta1), 2) * (M1 * M1 * (gamma + cos(2 * beta1)) + 2)) + (2 / tan(beta1)) * (2 * M1 * M1 * sin(2 * beta1)) * (M1 * M1 * gamma + M1 * M1 * cos(2 * beta1) + pow(M1 * sin(beta1), 2) + 1) / pow(M1 * M1 * (gamma + cos(2 * beta1)) + 2, 2);

			beta2 = beta1 - fn_beta1 / fn_prime_beta1;

			cout << "Iteration: " << itr << ". Convergence: " << abs(beta2 - beta1) << ". Beta: " << beta2 * 180 / PI << " DEG. \n";

			if (abs(beta2 - beta1) < crit) {
				cout << "Iteration completed at iteration " << itr << " with convergence " << abs(beta2 - beta1) << ".\n\n";
				break;
			}
			if (itr == 100) {
				cout << "[ERROR] Maxed out iteration. Adjust convergence criterion, initial guess or iteration number. Look through log and manual enter approx beta value if cycle of values occurs. \n\n";
				return -1;
			}
			beta1 = beta2;
		}

		shock_prop[2] = beta2 * 180 / PI; // Convert back to DEG
		M1 = shock_prop[0] * sin(beta2); // beta2 alr in RAD
	}
	else { // Oblique shock, beta known. Find theta
		if (shock_prop[2] == 0) {
			cout << "[ERROR] Shockwave angle cannot be zero! Ending program! \n\n";
			return -1;
		}
		// Find theta (even if theta value has been entered by user, it will be recal and overwritten for programming simplicity
		shock_prop[3] = atan((2 / tan(shock_prop[2] * PI / 180)) * ((pow(shock_prop[0] * sin(shock_prop[2] * PI / 180), 2) - 1) / (shock_prop[0] * shock_prop[0] * (gamma + cos(2 * (shock_prop[2] * PI / 180))) + 2)));
		shock_prop[3] = shock_prop[3] * 180 / PI;
		if (shock_prop[3] < 0) {
			cout << "[ERROR] For stated Mach Number, the shockwave angle " << shock_prop[2] << " DEG will not occer. Ending program! \n\n";
			return -1;
		}

		M1 = shock_prop[0] * sin(shock_prop[2] * PI / 180);
	}
	return M1;
} 



// SUB-FUNCTION is to calculate flow properties across expansion fan
void expand_fan_cal(double fan_prop[], int size, double GasProp[]) {
	/*fan_prop[0] = M1
	  fan_prop[1] = M2
	  fan_prop[2] = theta
	*/

	// Step 0: Check whether M1 > 1 abd other error conditions
	if (fan_prop[0] <= 1 && fan_prop[0] >= 0) {
		cout << "[ERROR] Mach Number 1 (M1) cannot be less than 1! Expansion fan unable to form in subsonic conditions! Ending program! \n\n";
		return;
	}
	if (fan_prop[1] <= 1 && fan_prop[1] >= 0) {
		cout << "[ERROR] Mach Number 2 (M2) cannot be less than 1! Expansion fan does not reduce Mach Number! Ending program! \n\n";
		return;
	}

	double gamma = GasProp[0];
	double theta = 0; // To convert to RAD as trigo fn in math.h is in RAD
	double v1 = 0; // v(M1)
	double v2 = 0; // v(M2)
	
	// Step 1: Finding M1
	if (fan_prop[0] < 0) {
		if (fan_prop[1] < 0 || fan_prop[2] < 0) {
			cout << "[ERROR] Mach Number 2 and Theta must be known to solve for Mach Number 1. Ending program!\n\n";
			return;
		}
		
		theta = fan_prop[2]; // in DEG --> For reason, see SUB-Fn expan_fan_v_cal

		v2 = expan_fan_v_cal(fan_prop[1], gamma, 1);
		if (v2 < 0) {
			return;
		}

		v1 = v2 - theta;

		fan_prop[0] = expan_fan_v_cal(v1, gamma, 2);
		if (fan_prop[0] < 0) {
			return;
		}
	}

	// Step 2: Finding M2
	if (fan_prop[1] < 0) {
		if (fan_prop[0] < 0 || fan_prop[2] < 0) {
			cout << "[ERROR] Mach Number 1 and Theta must be known to solve for Mach Number 2. Ending program!\n\n";
			return;
		}

		theta = fan_prop[2]; // in DEG --> For reason, see SUB-Fn expan_fan_v_cal

		v1 = expan_fan_v_cal(fan_prop[0], gamma, 1);
		if (v1 < 0) {
			return;
		}

		v2 = v1 + theta;

		fan_prop[1] = expan_fan_v_cal(v2, gamma, 2);
		
		if (fan_prop[1] < 0) {
			return;
		}
	}
	
	// Step 3: Finding theta
	if (fan_prop[2] < 0) {
		if (fan_prop[0] < 0 || fan_prop[1] < 0) {
			cout << "[ERROR] Mach Number 1 and 2 must be known to solve for theta. Ending program!\n\n";
			return;
		}

		v1 = expan_fan_v_cal(fan_prop[0], gamma, 1);
		v2 = expan_fan_v_cal(fan_prop[1], gamma, 1);
		if (v1 < 0 || v2 < 0) {
			return;
		}

		theta = v2 - v1;
		
		fan_prop[2] = theta; // theta in DEG --> For reason, see SUB-Fn expan_fan_v_cal
	}

	// Step 4: Print results to console
	cout << "***EXPANSION FAN CALCULATOR SOLUTION***\n\n";
	cout << "If value is 0, calculation on that flow property not performed.\n";
	cout << "Mach Number BEFORE shock (M1)........................... " << fan_prop[0] << "\n";
	cout << "Mach Number AFTER shock (M2)............................ " << fan_prop[1] << "\n";
	cout << "Flow / Turn / Wall angle (theta)........................ " << fan_prop[2] << " DEG\n";
	
	
	return;
}



// SUB-FUNCTION is to calculate v(M) for given M [MODE 1] or calculate M for given v(M) [MODE 2]
double expan_fan_v_cal(double given_var, double gamma, int mode) {
	double v = 0;
	double M = 0;
		
	if (mode == 1) { // Find v(M) for given M
		M = given_var;

		double temp1, temp2;
		temp1 = sqrt((gamma - 1) * (M * M - 1) / (gamma + 1));
		temp2 = sqrt(M * M - 1);

		v = sqrt((gamma + 1) / (gamma - 1)) * atan(temp1) - atan(temp2); // in RAD
		v = v * 180 / PI; // Convert to Deg, Reason why theta in SUB-Fn expan_fan_cal can be in DEG
		
		return v;
	}
	else if (mode == 2) { // Find M for given v(M). ISSUES HERE
		v = given_var;

		int check = -1;
		int itr = 1;
		double increment, M1, M2, f1, f2, temp1, temp2;

		cout << "Enter increment value. Default value: 0.01. A lower increment number would yield more accurate results but take longer time. \nIncrement value: ";
		cin >> increment;
		cout << "\n";

		M1 = 1;
		
		temp1 = sqrt((gamma - 1) * (M1 * M1 - 1) / (gamma + 1));
		temp2 = sqrt(M1 * M1 - 1);

		f1 = sqrt((gamma + 1) / (gamma - 1)) * atan(temp1) - atan(temp2) - (v * PI / 180); // in RAD. v is in DEG therefore need to be converted

		if (f1 < 0.0009 && f1 > -0.0009) {
			M = M1;
			return M;
		}
		
		// Itr method. Prandtl Mayer Fn, shift all variables to one side. Ideal --> Fn equates to zero --> solved for M. But unable, therefore, insert increments of M into Fn, once fn from -ve become +ve (or vice versa)
		// Fn has been "solved". Need to interpolate between the M for f1 and M for f2 --> get approx soln M.
		while (check == -1) {
			M1 = M1 + increment;

			temp1 = sqrt((gamma - 1) * (M1 * M1 - 1) / (gamma + 1));
			temp2 = sqrt(M1 * M1 - 1);

			f2 = sqrt((gamma + 1) / (gamma - 1)) * atan(temp1) - atan(temp2) - (v * PI / 180); // in RAD. v is in DEG therefore need to be converted

			cout << "Interation: " << itr << ". f1 value: " << f1 << " at Mach " << M1 - increment << ". f2 value: " << f2 << " at Mach " << M1 << ". \n";


			if (f1 < 0 && f2 > 0) {
				cout << "\n";
				cout << "Iteration stopped at: " << itr << ". f1 value: " << f1 << " at Mach " << M1 - increment << ". f2 value: " << f2 << " at Mach " << M1 << ". \n\n";
				check = 0; // exit loop
				break;
			}

			if (f1 > 0 && f2 < 0) {
				cout << "\n";
				cout << "Iteration stopped at: " << itr << ". f1 value: " << f1 << " at Mach " << M1 - increment << ". f2 value: " << f2 << " at Mach " << M1 << ". \n\n";
				check = 0; // exit loop
				break;
			}

			if (f2 == 0) {
				cout << "\n";
				cout << "Iteration stopped at: " << itr << ". f1 value: " << f1 << " at Mach " << M1 - increment << ". f2 value: " << f2 << " at Mach " << M1 << ". \n\n";
				check = 0; // exit loop
				return M1;
			}

			if (f2 < f1) {
				cout << "[ERROR] f2 < f1, not possible in Prandtl Mayer Fn. Ending program!\n";
				check = 0;
				return -1;
			}

			f1 = f2;
			itr = itr + 1;

		}
		M2 = M1;
		M1 = M1 - increment;

		// Interpolate to get M
		M = M2 - (f2 * (M2 - M1)) / (f2 - f1);

		return M;
	}

	return -1;
}



// SUB-FUNCTION is to check whether a converging nozzle is choked
void nozzle_con(double nozzle_prop[], int size, double GasProp[]) {
	/*
	nozzle_prop[0] = Pt
	nozzle_prop[1] = P(in)
	nozzle_prop[2] = M(in)
	nozzle_prop[3] = P(b)
	*/

	// Step 0: Check for errors
	if (nozzle_prop[3] == 0) {
		cout << "[ERROR] Back / Ambient Pressure cannot be ignored! Use Isentropic Calculator mode (Program Function 1) to find Back / Ambient Pressure if unknown. Ending program! \n\n";
		return;
	}
	if (nozzle_prop[0] <= 0) {
		if (nozzle_prop[1] > 0 && nozzle_prop[2] > 0) {
			// do nothing
		}
		else {
			cout << "[ERROR] BOTH nozzle entrance pressure and Mach Number must be known! Use Isentropic Calculator mode (Program Function 1) to find if unknown. Ending program! \n\n";
			return;
		}
	}

	double gamma = GasProp[0];
	double ratio, P_star, Pb;
	int choke_check = 0;

	// Step 1: If Pt unknown --> calculate and find Pt
	if (nozzle_prop[0] <= 0) {

		ratio = pow((1 + 0.5 * (gamma - 1) * pow(nozzle_prop[2], 2)), gamma / (gamma - 1));
		nozzle_prop[0] = ratio * nozzle_prop[1];
	}

	// Step 2: Find if nozzle choked
	ratio = pow((1 + 0.5 * (gamma - 1)), gamma / (gamma - 1)); // ratio of P*/Pt --> P/Pt when M = 1
	P_star = (1 / ratio) * nozzle_prop[0];

	Pb = nozzle_prop[3]; // Back pressure Pb

	if (abs(Pb - P_star) < PRSCRIT) { // These conditions placed ahead of others as prog will execute as such --> if top conditions met first, others below will be ignored. I.e if this cond placed below (Pb < P_star && Pb > 0) cond, it will not be executed as latter cond is met first and other cond below it will be ignored.
		// Reason why Pressure units standardise to MPa --> Easier to standardise (Pb +- 0.001) == P_star if just choke cond. If Pb == P_star --> need to be exact same down to 6 d.p --> impossible + unrealistic.
		// Just choked
		choke_check = 1;
	}
	else if (Pb > P_star) {
		// Not choked
		choke_check = 0;
	}
	else if (Pb < P_star && Pb > 0) {
		// CHoked
		choke_check = 2;
	}
	else if (Pb < 0) {
		// Just finding critical back pressure
		choke_check = -1;
		nozzle_prop[3] = 0;
	}
	/*
	if (Pb > P_star) {
		// Not choked
		choke_check = 0;
	}
	else if (abs(Pb - P_star) < PRSCRIT) {  
		// Reason why Pressure units standardise to MPa --> Easier to standardise (Pb +- 0.001) == P_star if just choke cond. If Pb == P_star --> need to be exact same down to 6 d.p --> impossible + unrealistic.
		// Just choked
		choke_check = 1;
	}
	else if (Pb < P_star && Pb > 0) {
		// Choked
		choke_check = 2;
	}
	else if (Pb < 0) {
		// Just finding critical back pressure
		choke_check = -1;
		nozzle_prop[3] = 0;
	}
	*/

	// Step 3: Print results to console
	cout << "***CONVERGING NOZZLE SOLUTION***\n\n";
	cout << "If value is 0, calculation on that flow property not performed.\n";
	cout << "Nozzle Total / Stagnation Pressure (Pt)................. " << nozzle_prop[0] << " MPa\n";
	cout << "Nozzle Entrance Pressure (P)............................ " << nozzle_prop[1] << " MPa\n";
	cout << "Nozzle Entrance Mach Number (M)......................... " << nozzle_prop[2] << " \n";
	cout << "Back / Ambient Pressure (Pb)............................ " << nozzle_prop[3] << " MPa\n\n";
	
	cout << "Critical choking Back / Ambient Pressure (P*)........... " << P_star << " MPa\n";
	cout << "Nozzle condition........................................ ";

	switch (choke_check) {
		case -1: // Just finding critical back pressure
			cout << "N/A";
			break;

		case 0: // Not choked
			cout << "NOT CHOKED";
			break;

		case 1: // Just choked
			cout << "AT CRITICAL CHOKE CONDITION";
			break;

		case 2: // Choke
			cout << "CHOKED";
			break;

	}
	cout << "\n\n";

	cout << "        [CHOKE]        |       [NOT CHOKE]          \n";
	cout << "----------------------------------------------> INCREASING Back Pressure \n";
	switch (choke_check) {
	case -1: // Just finding critical back pressure
		cout << "                       ^ P* = " << P_star << " MPa\n";
		break;

	case 0: // Not choke
		cout << "                       ^ P* = " << P_star << " MPa\n";
		cout << "                                  ^ Pb = " << nozzle_prop[3] << " MPa\n";
		cout << "\n";
		break;

	case 1: // Just choke
		cout << "                       ^ P* = " << P_star << " MPa\n";
		cout << "                       ^ Pb = " << nozzle_prop[3] << " MPa\n";
		cout << "\n";
		break;

	case 2: //Choke
		cout << "                       ^ P* = " << P_star << " MPa\n";
		cout << "        ^ Pb = " << nozzle_prop[3] << " MPa\n";
		cout << "\n";
		break;

	}

	return;
}



// SUB-FUNCTION is to check whether a converging-diverging nozzle is choked
void nozzle_con_div(double nozzle_prop[], int size, double GasProp[]) {
	/*
	nozzle_prop[0] = Pt_in
	nozzle_prop[1] = Tt_in
	nozzle_prop[2] = At
	nozzle_prop[3] = Ae
	nozzle_prop[4] = Pb
	*/

	// Step 0: Check for errors
	int i;
	for (i = 0; i <= 3;i++) {
		if (nozzle_prop[i] <= 0) {
			cout << "[ERROR] Nozzle flow properties underdefined! Ending program! \n\n";
			return;
		}
	}
	
	if (nozzle_prop[4] == 0) {
		cout << "[ERROR] Back pressure value cannot be zero! Ending program! \n\n";
		return;
	}

	if (nozzle_prop[2] >= nozzle_prop[3]) {
		cout << "[ERROR] Not a converging diverging nozzle! Ending program! \n\n";
		return;
	}


	// Step 1: Find critical back pressures
	double gamma = GasProp[0];
	double area_ratio, crit, MachItr1, MachItr2; // Ae/At = A/A*
	int itr;

	double crit_pressure[3] = { 0 }; // { full isen SUBSONIC choked flow , full isen SUPERSONIC choked flow , shock just at exit }
	double mach_soln[3] = { 0 }; // { Subsonic soln of A/A* , Supersonic soln of A/A* , Mach Number at exit if normal shock just at exit}

	// Step 1A: Find subsonic and supersonic soln of M for Ae/At = A/A*
	area_ratio = nozzle_prop[3] / nozzle_prop[2];

	mach_soln[0] = 0.001; // Initial guess for subsonic soln
	mach_soln[1] = 5.001; // Initial guess for supersonic soln

	
	cout << "Enter convergence criterion. Default value: 0.0001. A lower number would yield more accurate results. \nConvergence criterion: ";
	cin >> crit;
	cout << "\n";

	for (i = 0; i < 2; i++) {
		MachItr1 = mach_soln[i]; // Assign guess soln. 0.001 for subsonic, 5.001 for supersonic
		if (i == 0) {
			cout << "Finding critical SUBSONIC Mach Number at nozzle exit: \n";
			cout << "Iteration: 0. Convervegence: N/A. Mach Number Solution: " << MachItr1 << ". \n";
		}
		else if (i == 1) {
			cout << "Finding critical SUPERSONIC Mach Number at nozzle exit: \n";
			cout << "Iteration: 0. Convervegence: N/A. Mach Number Solution: " << MachItr1 << ". \n";
		}

		for (itr = 1; itr <= 100; itr++) {
			MachItr2 = MachItr1 - (0.5 * (gamma + 1) * pow(area_ratio, 2 * (gamma - 1) / (gamma + 1)) * pow(MachItr1, (gamma - 1) / (gamma + 1)) - 0.5 * (gamma - 1) * MachItr1 - 1) / (0.5 * (gamma - 1) * (pow(area_ratio, 2 * (gamma - 1) / (gamma + 1)) * pow(MachItr1, -2 / (gamma + 1)) - 1));
			cout << "Iteration: " << itr << ". Convergence: " << abs(MachItr2 - MachItr1) << ". Mach Number Solution: "<< sqrt(MachItr2) << ".\n";

			if (abs(MachItr2 - MachItr1) < crit) {
				cout << ">>Iteration completed at iteration " << itr << " with convergence " << abs(MachItr2 - MachItr1) << ".\n\n";
				break;
			}
			if (itr == 100) {
				cout << "[ERROR] Maxed out iteration. Adjust convergence criterion, initial guess or iteration number. FOR DEBUGGING. PROGRAM EXIT. \n\n";
				return;
			}
			MachItr1 = MachItr2;
		}
		mach_soln[i] = sqrt(MachItr2);

	}
	// Step 1B: Find crit back pressure of fully isentropic SUBSONIC choked flow using isentropic r/n --> mach_soln[0] & crit_pressure[0]
	crit_pressure[0] = nozzle_prop[0] / pow(1 + 0.5 * (gamma - 1) * mach_soln[0] * mach_soln[0], gamma / (gamma - 1));

	// Step 1C: Find crit back pressure of fully isentropic SUPERSONIC choked flow using isentropic r/n --> mach_soln[1] & crit_pressure[1]
	crit_pressure[1] = nozzle_prop[0] / pow(1 + 0.5 * (gamma - 1) * mach_soln[1] * mach_soln[1], gamma / (gamma - 1));

	// Step 1D: Find crit back pressure of choked flow whereby normal shock just at exit using normal shock r/n (M1 = mach_soln[1]; P1 = crit_pressure[1])--> mach_soln[2] & crit_pressure[2]
	mach_soln[2] = (1 + 0.5 * (gamma - 1) * mach_soln[1] * mach_soln[1]) / (gamma * mach_soln[1] * mach_soln[1] - 0.5 * (gamma - 1));
	mach_soln[2] = sqrt(mach_soln[2]);

	crit_pressure[2] = crit_pressure[1] * (1 + ((2 * gamma) / (gamma + 1)) * (mach_soln[1] * mach_soln[1] - 1));
	

	// Step 2: Find condition the nozzle is in
	int nozzle_cond_type = 0;

	if (abs(nozzle_prop[4] - crit_pressure[0]) < PRSCRIT) { // These conditions placed ahead of others as prog will execute as such --> if top conditions met first, others below will be ignored. I.e if this cond placed below (nozzle_prop[4] > crit_pressure[0]) cond, it will not be executed as latter cond is met first and other cond below it will be ignored.
		// Reason why Pressure units standardise to MPa --> Easier to standardise (Pb +- 0.001) == crit_pressure. If Pb == crit_pressure --> need to be exact same down to 6 d.p --> impossible + unrealistic.
		// Pb = Pb(d) --> Fully isentropic subsonic choked flow
		nozzle_cond_type = 2;
	}
	else if (abs(nozzle_prop[4] - crit_pressure[2]) < PRSCRIT) {
		// Reason why Pressure units standardise to MPa --> Easier to standardise (Pb +- 0.001) == crit_pressure. If Pb == crit_pressure --> need to be exact same down to 6 d.p --> impossible + unrealistic.
		// Pb = Pb(g) --> Normal shock just at exit of nozzle
		nozzle_cond_type = 4;
	}
	else if (abs(nozzle_prop[4] - crit_pressure[1]) < PRSCRIT) {
		// Reason why Pressure units standardise to MPa --> Easier to standardise (Pb +- 0.001) == crit_pressure. If Pb == crit_pressure --> need to be exact same down to 6 d.p --> impossible + unrealistic.
		// Pb = Pb(i) --> Perfect expanded nozzle. Fully isentropic supersonic choked flow
		nozzle_cond_type = 6;
	}
	else if (nozzle_prop[4] < 0) {
		// Only finding crit pressures
		nozzle_cond_type = -1;
	}
	else if (nozzle_prop[4] > crit_pressure[0]) {
		// Pb > Pb(d) --> Fully isentropic subsonic unchoked flow
		nozzle_cond_type = 1;
	}
	else if (nozzle_prop[4] < crit_pressure[0] && nozzle_prop[4] > crit_pressure[2]) {
		// Pb(g) < Pb < Pb(d) --> Normal shock within diverging section of nozzle
		nozzle_cond_type = 3;
	}
	else if (nozzle_prop[4] < crit_pressure[2] && nozzle_prop[4] > crit_pressure[1]) {
		// Pb(i) < Pb < Pb(g) --> Overexpanded nozzle. Oblique shock at exit of nozzle
		nozzle_cond_type = 5;
	}
	else if (nozzle_prop[4] < crit_pressure[1]) {
		// Pb < Pb(i) --> Underexpanded nozzle. Expansion fan at exit of nozzle
		nozzle_cond_type = 7;
	}

	if (nozzle_cond_type == 0) {
		cout << "[ERROR] Unable to assign nozzle condition! Ending program! \n\n";
		return;
	}

	/*
	if (nozzle_prop[4] < 0) {
		// Only finding crit pressures
		nozzle_cond_type = -1;
	}
	else if (nozzle_prop[4] > crit_pressure[0]) {
		// Pb > Pb(d) --> Fully isentropic subsonic unchoked flow
		nozzle_cond_type = 1;
	}
	else if (abs(nozzle_prop[4] - crit_pressure[0]) < PRSCRIT) {
		// Reason why Pressure units standardise to MPa --> Easier to standardise (Pb +- 0.001) == crit_pressire. If Pb == crit_pressure --> need to be exact same down to 6 d.p --> impossible + unrealistic.
		// Pb = Pb(d) --> Fully isentropic subsonic choked flow
		nozzle_cond_type = 2;
	}
	else if (nozzle_prop[4] < crit_pressure[0] && nozzle_prop[4] > crit_pressure[2]) {
		// Pb(g) < Pb < Pb(d) --> Normal shock within diverging section of nozzle
		nozzle_cond_type = 3;
	}
	else if (abs(nozzle_prop[4] - crit_pressure[2]) < PRSCRIT) {
		// Reason why Pressure units standardise to MPa --> Easier to standardise (Pb +- 0.001) == crit_pressire. If Pb == crit_pressure --> need to be exact same down to 6 d.p --> impossible + unrealistic.
		// Pb = Pb(g) --> Normal shock just at exit of nozzle
		nozzle_cond_type = 4;
	}
	else if (nozzle_prop[4] < crit_pressure[2] && nozzle_prop[4] > crit_pressure[1]) {
		// Pb(i) < Pb < Pb(g) --> Overexpanded nozzle. Oblique shock at exit of nozzle
		nozzle_cond_type = 5;
	}
	else if (abs(nozzle_prop[4] - crit_pressure[1]) < PRSCRIT) {
		// Reason why Pressure units standardise to MPa --> Easier to standardise (Pb +- 0.001) == crit_pressire. If Pb == crit_pressure --> need to be exact same down to 6 d.p --> impossible + unrealistic.
		// Pb = Pb(i) --> Perfect expanded nozzle. Fully isentropic supersonic choked flow
		nozzle_cond_type = 6;
	}
	else if (nozzle_prop[4] < crit_pressure[1]) {
		// Pb < Pb(i) --> Underexpanded nozzle. Expansion fan at exit of nozzle
		nozzle_cond_type = 7;
	}
	*/
	
	
	// Step 3: Find exit Mach Number, Temperature and Total Pressure, if Pb != -1
	double Me, Te, Tte, Pe, Pte, a, b, c, Me1, Me2;

	Me = 0; // Initialising the variables to prevent errors
	Te = 0;
	Tte = 0;
	Pe = 0;
	Pte = 0;

	a = 0;
	b = 0;
	c = 0;

	Me1 = 0;
	Me2 = 0;
	
	switch (nozzle_cond_type) {
		case -1: // Finding crit pressures only
			// Do nothing
			break;

		case 1: // Fully isentropic subsonic flow (Pb = Pe)
			Me = sqrt((pow(nozzle_prop[0] / nozzle_prop[4], (gamma - 1) / gamma) - 1) * 2 / (gamma - 1));
			Pte = nozzle_prop[0];
			Pe = nozzle_prop[4];
			Tte = nozzle_prop[1];
			Te = nozzle_prop[1] / (1 + 0.5 * (gamma - 1) * Me * Me);
			break;

		case 2: // Fully isentropic subsonic choked flow (Pb = Pe)
			Me = mach_soln[0];
			Pte = crit_pressure[0] * pow(1 + 0.5 * (gamma - 1) * Me * Me, gamma / (gamma - 1));
			Pe = crit_pressure[0];
			Tte = nozzle_prop[1];
			Te = nozzle_prop[1] / (1 + 0.5*(gamma - 1) * Me * Me);
			break;

		case 3: // Normal shock in diverging section of nozzle (Pb = Pe)
			// For info on how Me was calculated, see ME5309 Assignment 1
			a = 0.5 * (gamma - 1);
			b = 1;
			c = -1 * (1 + 0.5 * (gamma - 1)) * (pow(nozzle_prop[2] / nozzle_prop[3], 2)) * pow((nozzle_prop[0] / pow(1 + 0.5 * (gamma - 1), gamma / (gamma - 1))) / nozzle_prop[4], 2);

			Me1 = (-1 * b + sqrt(b * b - 4 * a * c)) / (2 * a);
			Me2 = (-1 * b - sqrt(b * b - 4 * a * c)) / (2 * a);

			if (Me1 > 0 && Me2 < 0) {
				Me = sqrt(Me1);
			}
			else if (Me2 > 0 && Me1 < 0) {
				Me = sqrt(Me2);
			} 
			else {
				if (Me1 > 0 && Me2 > 0) {
					if (Me1 > Me2) {
						Me = sqrt(Me2);
					}
					else { // Me2 > Me1 > 0
						Me = sqrt(Me1);
					}
				}
				else { // Me1 < 0 && Me2 < 0
					cout << "[ERROR] Imaginary Roots calculated for Exit Mach Number when normal shock is present in diverging section of nozzle. Ending program! \n\n";
					return;
				}
			}

			Pte = nozzle_prop[4] * pow(1 + 0.5 * (gamma - 1) * Me * Me, gamma / (gamma - 1));
			Pe = nozzle_prop[4];
			Tte = nozzle_prop[1];
			Te = nozzle_prop[1] / (1 + 0.5 * (gamma - 1) * Me * Me);
			
			break;

		case 4: // Normal shock just at exit (Pb != Pe)
			Me = mach_soln[2];
			Pte = crit_pressure[2] * pow(1 + 0.5 * (gamma - 1) * Me * Me, gamma / (gamma - 1));
			Pe = crit_pressure[2];
			Tte = nozzle_prop[1];
			Te = nozzle_prop[1] / (1 + 0.5 * (gamma - 1) * Me * Me);
			break;

		case 5: // Over expanded nozzle. Oblique shock (Pb != Pe)
			Me = mach_soln[1];
			Pte = crit_pressure[1] * pow(1 + 0.5 * (gamma - 1) * Me * Me, gamma / (gamma - 1));
			Pe = crit_pressure[1];
			Tte = nozzle_prop[1];
			Te = nozzle_prop[1] / (1 + 0.5 * (gamma - 1) * Me * Me);
			break;
		case 6: // Perfectly expanded nozzle; Fully isentropic supersoninc choked flow (Pb = Pe)
			Me = mach_soln[1];
			Pte = crit_pressure[1] * pow(1 + 0.5 * (gamma - 1) * Me * Me, gamma / (gamma - 1));
			Pe = crit_pressure[1];
			Tte = nozzle_prop[1];
			Te = nozzle_prop[1] / (1 + 0.5 * (gamma - 1) * Me * Me);
			break;

		case 7: // Under expanded nozzle. Expansion fan (Pb != Pe)
			Me = mach_soln[1];
			Pte = crit_pressure[1] * pow(1 + 0.5 * (gamma - 1) * Me * Me, gamma / (gamma - 1));
			Pe = crit_pressure[1];
			Tte = nozzle_prop[1];
			Te = nozzle_prop[1] / (1 + 0.5 * (gamma - 1) * Me * Me);
			break;
			
	}

	// Step 4: Find location of shockwave if normal shock present in the diverging section of the nozzle, if Pb != -1
	double A_shock = 0;
	double M1 = 0; // Shock jsut before normal shock

	if (nozzle_cond_type == 3) {
		// Step 4A: Find M1, Mach Number just before normal shock in diverging section of nozzle
		double ratio, step, f1, f2;
		int checkLoop = -1;
		
		itr = 1;
		
		ratio = Pte / nozzle_prop[0]; // ratio = Pt2/Pt1
		
		step = 0.01;
		M1 = 1;

		f1 = pow(((gamma + 1) * M1 * M1) / (2 + (gamma - 1) * M1 * M1), gamma / (gamma - 1)) * pow((gamma + 1) / (2 * gamma * M1 * M1 - (gamma - 1)), 1 / (gamma - 1)) - ratio;

		if (f1 == 0) {
			M1 = 1;
		}
		else {
			cout << "Finding Mach Number for given Pt2/Pt1 ratio. Pt2 is Total Pressure Nozzle exit, Pt1 is Total Pressure Nozzle inlet.\n";
			while (checkLoop == -1) {
				M1 = M1 + step;
				f2 = pow(((gamma + 1) * M1 * M1) / (2 + (gamma - 1) * M1 * M1), gamma / (gamma - 1)) * pow((gamma + 1) / (2 * gamma * M1 * M1 - (gamma - 1)), 1 / (gamma - 1)) - ratio;

				cout << "Iteration: " << itr << ". f1 value " << f1 << " at Mach " << M1 - step << ". f2 value " << f2 << " at Mach " << M1 << ". \n";

				if (f1 < 0 && f2 > 0) {
					checkLoop = 1;
					cout << ">>Iteration stopped at: " << itr << ". f1 value: " << f1 << " at Mach " << M1 - step << ". f2 value: " << f2 << " at Mach " << M1 << ". \n\n";
					break;
				}
				else if (f1 > 0 && f2 < 0) {
					checkLoop = 1;
					cout << ">>Iteration stopped at: " << itr << ". f1 value: " << f1 << " at Mach " << M1 - step << ". f2 value: " << f2 << " at Mach " << M1 << ". \n\n";
					break;
				}

				f1 = f2;
				itr = itr++;
			}
			
			M1 = (M1 - step) - (f1 * -1 * step) / (f1 - f2);
		}

		// Step 4B: Find Area of diverging section where Shock occurs
		A_shock = nozzle_prop[2] * sqrt((1 / (M1 * M1)) * pow((2 / (gamma + 1)) * (1 + 0.5 * (gamma - 1) * M1 * M1), (gamma + 1) / (gamma - 1)));
	}
	
	// Step 5: Print results to console
	cout << "***CONVERGING-DIVERGING NOZZLE SOLUTION***\n\n";
	cout << "[INPUT VALUES]\n";
	cout << "If Back / Ambient Pressure is -1, program finding critical back pressures only.\n";
	cout << "Nozzle entrance Total / Stagnation Pressure (Pt_in)........... " << nozzle_prop[0] << " MPa\n";
	cout << "Nozzle entrance Total / Stagnation Temperature (Tt_in)........ " << nozzle_prop[1] << " K\n";
	cout << "Nozzle Throat Area At (m^2)................................... " << nozzle_prop[2] << " m^2\n";
	cout << "Nozzle Exit Area, Ae (m^2).................................... " << nozzle_prop[3] << " m^2\n";
	cout << "Nozzle exit Back / Ambient Pressure (Pb)...................... " << nozzle_prop[4] << " m^2\n\n";

	cout << "[OUTPUT VALUES - CRITICAL BACK PRESSURES]\n";
	cout << "Fully isentropic SUBSONIC choked flow......................... " << crit_pressure[0] << " MPa w/ Mach " << mach_soln[0] << " at nozzle exit\n";
	cout << "Normal shock formed at nozzle exit............................ " << crit_pressure[2] << " MPa w/ Mach " << mach_soln[2] << " at nozzle exit\n";
	cout << "Perfectly expanded nozzle / Fully isentropic SUPERSONIC flow.. " << crit_pressure[1] << " MPa w/ Mach " << mach_soln[1] << " at nozzle exit\n\n";

	cout << "[OUTPUT VALUES - NOZZLE CONDITION]\n";
	cout << "Nozzle condition.............................................. ";
	switch (nozzle_cond_type) {
		case -1: // Finding crit pressure only
			cout << "N/A";
			break;

		case 1: // Subsonic unchoked
			cout << "FULLY SUBSONIC ISENTROPIC FLOW. UNCHOKED.";
			break;

		case 2: // Subsonic choke
			cout << "FULLY SUBSONIC ISENTROPIC FLOW. CHOKED.";
			break;

		case 3: // Normal shock in diverging section
			cout << "NORMAL SHOCK at DIVERGING SECTION OF NOZZLE. CHOKED.";
			break;

		case 4: // Normal shock just at exit
			cout << "NORMAL SHOCK at NOZZLE EXIT. CHOKED.";
			break;
			
		case 5: // Overexpanded. Oblique shock
			cout << "OVER EXPANDED NOZZLE. OBLIQUE SHOCK at NOZZLE EXIT. CHOKED.";
			break;

		case 6: // Perfect expanded. Fully isentropic supersonic
			cout << "PERFECT EXPANDED NOZZLE. FULLY ISENTROPIC SUPERSONIC ISENTROPIC FLOW. CHOKED.";
			break;

		case 7: // Underexpanded. Expansion fan
			cout << "UNDER EXPANDED NOZZLE. EXPANSION FAN at NOZZLE EXIT. CHOKED.";
			break;
	}
	cout << "\n\n";

	cout << "                        [CHOKED]                       |  [UNCHOKED]  \n";
	cout << "  [U/EXPAND]  |  [O/EXPAND]  |  [N SHOCK W/IN NOZZLE]  |  [SUBSONIC]  \n";
	cout << "---------------------------------------------------------------------------------> INCREASING Back Pressure \n";
	cout << "              ^ P/EXPAND: " << crit_pressure[1] << " MPa\n";
	cout << "                             ^ N/SHOCK NOZZLE EXIT: " << crit_pressure[2] << " MPa\n";
	cout << "                                                       ^ SUBSONIC ISEN CHOKE: " << crit_pressure[0] << " MPa\n";

	switch (nozzle_cond_type) {
	case -1:
		// Do nothing
		break;
	case 1:
		cout << "                                                               ^ BACK PRESSURE: " << nozzle_prop[4] << " MPa\n";
		break;
	case 2:
		cout << "                                                       ^ BACK PRESSURE: " << nozzle_prop[4] << " MPa\n";
		break;
	case 3:
		cout << "                                          ^ BACK PRESSURE: " << nozzle_prop[4] << " MPa\n";
		break;
	case 4:
		cout << "                             ^ BACK PRESSURE: " << nozzle_prop[4] << " MPa\n";
		break;
	case 5:
		cout << "                      ^ BACK PRESSURE: " << nozzle_prop[4] << " MPa\n";
		break;
	case 6:
		cout << "              ^ BACK PRESSURE: " << nozzle_prop[4] << " MPa\n";
		break;
	case 7:
		cout << "      ^ BACK PRESSURE: " << nozzle_prop[4] << " MPa\n";
		break;
	}
	
	cout << "\n";

	cout << "[OUTPUT VALUES] - NOZZLE EXIT FLOW CONDITION\n";
	cout << "Nozzle Exit Mach Number (Me).................................. " << Me << " \n";
	cout << "Nozzle Exit Total Pressure (Pt_exit).......................... " << Pte << " MPa\n";
	cout << "Nozzle Exit Pressure (P_exit)................................. " << Pe << " MPa\n";
	cout << "Nozzle Exit Total Temperature (Tt_exit)....................... " << Tte << " K\n";
	cout << "Nozzle Exit Temperature (T_exit).............................. " << Te << " K\n\n";
	
	if (nozzle_cond_type == 3) {
		cout << "Normal shock nozzle diverging section area.................... " << A_shock << " m^2\n";
		cout << "Mach Number before normal shock............................... " << M1 << "\n";
	}
	else if (nozzle_cond_type == 4) {
		cout << "Normal shock nozzle diverging section area.................... " << nozzle_prop[3] << " m^2\n";
		cout << "Mach Number before normal shock............................... " << mach_soln[1] << "\n";
	}
	else {
		cout << "Normal shock nozzle diverging section area.................... N/A m^2\n";
		cout << "Mach Number before normal shock............................... N/A\n";
	}

	cout << "\n";

	cout << "[OUTPUT VALUES] - REMARKS\n";
	if (nozzle_cond_type == 5) { // Over expanded nozzle - Oblique shock at nozzle exit
		cout << "OVER EXPANDED NOZZLE. OBLIQUE SHOCK AT NOZZLE EXIT. NOTE.\n";
		cout << "To find Flow Condition after Nozzle Exit, \n";
		cout << "> Re-run this program. Use Shockwave / Expansion fan calculator (PROGRAM FUNCTION 2), Normal/Oblique Shockwave mode (SUB FUNCTION 1).\n\n";

		cout << "> Enter M1 value as: " << mach_soln[1] << " (Fully isentropic SUPERSONIC flow Mach Number solution / Nozzle Exit Mach Number)\n";
		cout << "> Enter M2 value as: -1 (solution is Mach Number after Nozzle Exit, after shock)\n\n";
		
		cout << "> Enter flow / wall angle and/or shockwave angle if known.\n\n";
		
		cout << "> Enter T1 value as: " << Te << " K (Nozzle Exit Temperature)\n";
		cout << "> Enter T2 value as: -1 (solution is temperature after Nozzle Exit, after shock)\n\n";
		
		cout << "> Total Temperature value constant: " << Tte << " K (Total temperature at nozzle inlet same as outlet)\n\n";

		cout << "> Enter P1 value as: " << Pe << " MPa (Nozzle Exit Pressure)\n";
		cout << "> Enter P2 value as: " << nozzle_prop[4] << " MPa (Back / Ambient Pressure)\n\n";

		cout << "> Enter Pt1 value as: " << Pte << " MPa (Nozzle Exit Total Pressure)\n";
		cout << "> Enter Pt2 value as: -1 (solution is Total Pressure after Nozzle Exit, after shock) \n";
	}
	else if (nozzle_cond_type == 7) { // Under expanded nozzle - Expansion fan at nozzle exit
		cout << "UNDER EXPANDED NOZZLE. EXPANSION FAN AT NOZZLE EXIT. NOTE.\n";
		cout << "To find Flow Condition after Nozzle Exit, \n";
		cout << "> Re-run this program. Use Shockwave / Expansion fan calculator (PROGRAM FUNCTION 2), Expansion fan mode (SUB FUNCTION 2).\n\n";

		cout << "> Enter M1 value as: " << mach_soln[1] << " (Fully isentropic SUPERSONIC flow Mach Number solution / Nozzle Exit Mach Number)\n";
		cout << "> Enter M2 value as: -1 (solution is Mach Number after Nozzle Exit, after shock)\n\n";

		cout << "> Enter flow / wall angle.\n\n";

		cout << "With M2 value, use Isentropic Calculator (PROGRAM FUNCTION 1) to find other flow properties after Nozzle Exit.\n";
		cout << "Note that Total Temperature and Total Pressure constant accross expansion fan. Therefore, Tt and Pt value after nozzle same as at nozzle exit.\n";
	}
	else if (nozzle_cond_type == 1 || nozzle_cond_type == 2 || nozzle_cond_type == 3 || nozzle_cond_type == 6) {
		cout << "NOTE. Nozzle Exit pressure same as Back / Ambient Pressure.\n";
	}
	else {
		cout << "N/A\n";
	}
	cout << "\n";
	return;
}