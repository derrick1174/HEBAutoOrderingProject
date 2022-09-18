/**************************************************************
HEBAutoMain.cpp by Derrick Miller

Purpose:
	This program was coded as an example implementation of an idea I had to improve HEB's Curbside
	application, a web and mobile service that allows ordering of groceries from a store that are then
	brought out to your car and loaded by store teammembers. This program builds on the existing Curbside
	system by allowing existing users to set up automatic order scheduling on a weekly or bi-weekly basis.
	This program would be run as the sole program on a dedicated computer at a central computing location,
	and would handle all HEB customers using this automation system at once.


Input: 
	The program accepts no input. The program stores important information in an Excel or csv file, which may exist
	before the program starts, but will be created if no such file already exists.

Results:
	Automatically sends new orders to the existing HEB Curbside system, removing grocery ordering as a weekly task
	for users that have opted-in to this program.
**************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "HEBAutoMain.h"
#include <ctime>

//check operating system (for sleep function)
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

void main() {


	//create the file that all automation information is stored inside of, unless it already exists
	fstream outFile;
	
	outFile.open("HEBAutoOrders.csv");
	if (outFile.is_open()) {
		cout << "File has been opened" << endl;
	}
	else {
		//file does not exist
		cout << "File has been created" << endl;
		outFile.open("HEBAutoOrders.csv", ios::out);
		outFile << "userID,storeID,orderDate,idealTimeslots[24],acceptableTimeslots[24],(itemID,unitQuantity)[SHOPPINGCARTITEMLIMIT]\n";
	}
	outFile.close();

	
	while (1) {
		//check for users queued to sign up or withdraw from automatic orders
		signUps();
		//check for users queued resolved notifications
		//check for negative confirmations, cancelling those orders if they are denied
		userNotifications();
		//wait until midnight, at which point we check to see if we need to schedule a new order for next week or the week after
		midnightScheduler();

		//waits an arbitrary time in seconds, before looping
		Sleep(60);
	}
}


/****** midnightScehduler ***********
Purpose:
	This function is responsible for automatically adding new orders. When called shortly after midnight, 
	new orders for all users that had an order the previous day are passed to the curbside system.

Format for HEBAutoOrders.csv: userIDNum,storeID,idealTimeslots(24 booleans each separated by commas),acceptableTimeslots(24 booleans each separated by commas),cartItemID1,cartItemQuantity1,cartItemID2,cartItemQuantity2,cartItemID3,cartItemQuantity3...\n
	ex. 123456789,10450,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,0,01062,1,10732,4,....\n
						^idealTimeslots[0]						        ^acceptableTimeslots[0]							^cartItemID1

*************************************/
void midnightScheduler() {

	//check if it is midnight or close to midnight
	//if it is, check if we have already successfully run this method this day
	//if we have, this was a redundant call, exit
	//if we have not and it is midnight:
		//check that the information file exists
		//skip first line in the HEBAutoOrders file, as it is a header
		//schedule all users that had a previous order today for their order next week (or two weeks from now) by:
			//validating the timeslot, checking user-requested timeslots
			//pass information from the main information file to curbsideOrderPlacer
			//add a notification to the user account on a success (whether this notification goes to the user is handled by the curbside app)
			//if there is an error and we need to notify the user, add a notification to their account (the notification should be handled by their phone app or HEB's email system shortly after it is added to the account)
	

}

/******* signUps ******************
Purpose:
	Handles new users signing up for this program.
	Reads from signingUpFile.csv, which would be handled by the HEB Curbside App, which should also validate
	the user's userID and storeID before allowing this information to be added to the signingUpFile.

Format for signingUpFile.csv: userIDNum,storeID,orderDay,orderInterval,idealTimeslots(24 booleans each separated by commas),acceptableTimeslots(24 booleans each separated by commas),cartItemID1,cartItemQuantity1,cartItemID2,cartItemQuantity2,cartItemID3,cartItemQuantity3...\n
	ex. 123456789,10450,orderDay,orderInterval,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0(,)0,0,0,1,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,0,01062,1,10732,4,....\n
											   ^idealTimeslots[0]						         ^acceptableTimeslots[0]							^cartItemID1
***********************************/
void signUps() {
	//read information of new signup
	shoppingCartItem cartItem;
	list<shoppingCartItem> cart;
	fstream mainFile;
	fstream signingUpFile;
	
	string IDtemp;//holds info from getline, read in as a string before being converted into an integer as a part of thisUserAccount
	string storeIDtemp;//holds info from getline, read in as a string before being converted into an integer for storeID
	string orderDayTemp;//holds info from getline, read in as a string before being converted into a date format
	string orderIntervalTemp;//holds info from getline, read in as a string before being converted into an integer
	string inBoolTemp;//holds info from getline, read in as a string before being converted into a boolean
	string restOfLine;//temporary storage for the rest of line, from which inCartItemTemp and inCartQuantityTemp are parsed
	string inCartItemTemp;//holds info from getline, read in as a string before being converted into an integer as a part of cart
	string inCartQuantityTemp;//holds info from getline, read in as a string before being converted into an integer as a part of cart
	string eTemp;//error handling only; will hold the rest of the failed line to output to cout
	int userIDNum;
	int storeIDNum;
	int orderDay;//0-6,sunday-saturday
	int orderInterval;//how many days between automatic orders; 6 is weekly, 13 is biweekly, etc.
	int itemTemp;
	int quantityTemp;
	bool idealTimeslots[24];//reminder that index 0 is midnight
	bool acceptableTimeslots[24];//note, overlap with idealTimeslots is irrelevant, except that overlapping timeslots will make an extra call to availableTimeslots each


	signingUpFile.open("signingUpFile.csv");//read and write

	//read individual lines from signingUpFile per user
	while (!signingUpFile.eof()) {
		//read into thisUserAccount from signingUpFile, format: userIDNum,storeID,idealTimeslots(24 booleans separated by commas),acceptableTimeslots(24 booleans separated by commas),cartItemID1,cartItemQuantity1,cartItemID2,cartItemQuantity2,cartItemID3,cartItemQuantity3...\n
		getline(signingUpFile, IDtemp, ',');

		//this try/catch of the code assumes that HEB user IDs are integers only
		try {
			// Code that could throw an exception
			userIDNum = stoi(IDtemp);
		}
		catch (const invalid_argument e) {
			//cerr << e.what();
			cout << "A user in the signingUpFile had a userID that could not be converted into an integer\n";
			cout << "Details: failed userID: " << IDtemp << "\n";
			getline(signingUpFile, eTemp);
			cout <<	"Rest of line: " << eTemp << "\n";
			cout << "Skipping this sign up...\n";
			continue;
		}
		
		//check if the user information is valid (information that is required for automation only; this is not an account password validation)
			//includes existing userIDNum, cart has at least one item
		if (userIDNum == NULL) {
			cout << "A userID in signingUpFile was found to be NULL!\n";
			getline(signingUpFile, eTemp);
			cout << "Rest of line: " << eTemp << "\n";
			cout << "Skipping this sign up...\n";
			continue;

		}
		else {
			//	check this id doesn't exist elsewhere in the main info file (only one auto-order allowed)
			
			//	add this signup to HEBAutoOrders.csv
			getline(signingUpFile, IDtemp, ',');//get the storeID

			//this try/catch of the code assumes that HEB store IDs and HEB user IDS are integers only
			try {
				// Code that could throw an exception
				storeIDNum = stoi(storeIDtemp);
			}
			catch (const invalid_argument e) {
				//cerr << e.what();
				cout << "A storeID in the signingUpFile had a storeID that could not be converted into an integer\n";
				cout << "Details: failed storeID: " << storeIDtemp << "\n";
				cout << "userID preceeding the failure: " << userIDNum << "\n";
				getline(signingUpFile, eTemp);
				cout << "Rest of line: " << eTemp << "\n";
				cout << "Skipping this sign up...\n";
				continue;
			}
			//need to validate storeID before continuing, but as this is an example, it is assumed the storeID is correct in the signingUpFile

			//get orderDay
			getline(signingUpFile,orderDayTemp,',');
			orderDay = stoi(orderDayTemp);//TODO:error handling //TODO:needs to slice only the day of the week out of this
			//TODO:check that orderDay is 0-6 (sun-sat)

			//get orderInterval
			getline(signingUpFile, orderIntervalTemp, ',');
			orderInterval = stoi(orderIntervalTemp);//TODO:error handling
			//TODO:check that orderInterval complies with arbitrary interval limiting; ex. HEB only allows weekly or biweekly auto-ordering
 
			//get idealTimeslots
			for (int i = 0; i < 24; i++) {
				//TODO:error handling
				getline(signingUpFile, inBoolTemp, ',');
				idealTimeslots[i] = stoi(inBoolTemp);
			}

			//getacceptableTimeslots
			for (int i = 0; i < 24; i++) {
				//TODO:error handling
				getline(signingUpFile, inBoolTemp, ',');
				acceptableTimeslots[i] = stoi(inBoolTemp);
			}

			//rest of line, should be items in shopping cart and their quantities, until \n or SHOPPINGCARTITEMLIMIT is reached
			getline(signingUpFile, restOfLine);
			stringstream restOfLineSS(restOfLine);//converts restOfLine into a stringstream so getline can operate on the remainder of the line
			while (!restOfLineSS.eof()) {//TODO:make sure we are not trying to exceed SHOPPINGCARTITEMLIMIT
				getline(restOfLineSS, inCartItemTemp, ',');//TODO:error handling
				getline(restOfLineSS, inCartQuantityTemp, ',');//TODO:error handling
				cartItem.itemID = stoi(inCartItemTemp);//TODO:error handling
				cartItem.unitQuantity = stoi(inCartQuantityTemp);//TODO:error handling
				cart.push_front(cartItem);
			}
			mainFile.open("HEBAutoOrders.csv", ios::out);//write only
			mainFile << userIDNum << "," << storeIDNum << "," << orderDay << "," << orderInterval << ",";
			//append idealTimeslots(24 booleans each separated by commas),acceptableTimeslots(24 booleans each separated by commas),cartItemID1,cartItemQuantity1,cartItemID2,cartItemQuantity2,cartItemID3,cartItemQuantity3...\n
			//append idealTimeslots[24]
			for (int i = 0; i < 24; i++) {
				mainFile << idealTimeslots[i] << ',';
			}
			//append acceptableTimeslots[24]
			for (int i = 0; i < 24; i++) {
				mainFile << acceptableTimeslots[i] << ',';
			}
			//append shopping cart items and quantities
			for (auto itr = cart.begin(); itr != cart.end(); itr++) {
				cartItem = cart.front();//take first item from cart
				itemTemp = cartItem.itemID;
				quantityTemp = cartItem.unitQuantity;
				mainFile << itemTemp << ',' << quantityTemp;
				if (!cart.empty()) {
					//add commas if this is not the last token
					mainFile << ',';
				} else { 
					//do not add a comma after the last token on the line
					mainFile << "\n";
				}
				cart.pop_front();//delete first item from cart
			}
			
			mainFile.close();
		}
	}
}

/****** timeslotChecker ***********
Purpose:
	takes idealTimeslots and acceptableTimeslots and asks existing HEB Curbside systems if a timeslot is free enough
	for an order to be placed in that timeslot, then will return the timeslot that is free as an index, or an error
	if no timeslot is available

Input:
	bool theseTimeslots[24]		range of timeslots that are being checked. Indices correlate to hours in a day
	int userIDNum				related user to this inquiry. Necessary to reserve the timeslot so no other order can conflict over remaining spots in a particular spot at a particular store
	int storeIDNum				related storeID to this inquiry. Different stores have different availabilities
	int orderDay				date of the requested inquiry

Returns:
	Timeslot index for this order that is free as an index or -1 if no free timeslots are available within acceptable hours
*************************************/
int timeslotChecker(bool theseTimeslots[24], int userIDNum, int storeIDNum, int orderDay) {

}

/****** userNotifcations *********** 
Purpose:
	This function handles the queue of user responses to notifications. The queue is generated by the Curbside system,
	generally as a result of users replying to notifications sent to them on their HEB Curbside phone app.



*************************************/
void userNotifications() {

	list<int> userIDNums;//holds all userIDNums pulled from file
	string userIDTemp;//string version of userIDNum read in from file
	int userIDNum;
	string restOfLine;
	fstream mainFile;
	mainFile.open("HEBAutoOrders.csv", ios::in);
	getline(mainFile, restOfLine);//skip header
	
	//gather all userIDs from the main automation file
	while (!mainFile.eof()) {
		getline(mainFile, userIDTemp, ',');//grab userIDNum from this line
		getline(mainFile, restOfLine);//skip the rest of this line
		try {
			userIDNum = stoi(userIDTemp);
			userIDNums.push_front(userIDNum);
		}
		catch(const invalid_argument e) {

			cout << "A user in the HEBAutoOrders file had a userID that could not be converted into an integer while resolving their notifications\n";
			cout << "Details: failed userID: " << userIDTemp << "\n";
			cout << "Rest of line: " << restOfLine << "\n";
			cout << "Skipping this notification...\n";
			continue;
		}
	}
	mainFile.close();

	//check each userIDNum for notifications
	//for each i in userIDNums{
		//while(!getNextUserNotification(userIDNums[i]) == 0){
			// int thisNotification = getNextUserNotification(userIDNums[i]);

			// handle it
			// if thisNotification == 1{
				// thisUser.notifications.push_front("Automatic order for " + to_string(orderDay) + " at " + to_string(timeslotIndex) + " cancelled successfully.")
			// }
			// if thisNotification fits date+requestedTimeslot+storeID format (check getNextUserNotification's documentation){
				// parse orderDay, timeslotIndex, and storeID from thisNotification (check getNextUserNotification's documentation)
				// check that this day and timeslot are available
				// int result = isFreeTimeslot(storeID, orderDay, timeslotIndex);
				// if success: updateUserNotificationDatabase(userIDNums[i], Automatic order for " + to_string(orderDay) + " at " + to_string(timeslotIndex) + " ordered successfully.", "add");
				// if failure: updateUserNotificationDatabase(userIDNums[i], Automatic order for " + to_string(orderDay) + " at " + to_string(timeslotIndex) + " failed due to full timeslot.", "add")
			// }
			//
		//}
	//}
	
}

/****** updateUserNotificationDatabase ***********
This function is code that already exists at HEB as part of the Curbside system.

Purpose:
	Updates the central HEB database of user notifications

Inputs:
	int thisUserIDNum				int of a userIDNum
	string thisNotification			must be a notification that can be added to a user account. These have a pre-established format.
	string actionType				must be either "add" or "remove"
*************************************/
void updateUserNotificationDatabase(int thisUserIDNum, string thisNotification, string actionType) {
	//this code updates a central database of user notifications, which is outside the scope of this project
	
	//if(actionType == "add"){
		//append thisNotification to thisUserIDNum's list of notifications
		//thisUserAtHEBCentralDatabase.notifications.push_front(thisNotification);
	//}
	//if (actionType == "remove") {
		//remove thisNotification from thisUserIDNum's list of notifications
		//thisUserAtHEBCentralDatabase.notifications.remove(thisNotification);
	//}
}

/****** getNextUserNotification ***********
This function is code that already exists at HEB as part of the Curbside system.

Purpose:
	Gets the next pending task that users have responded to as part of the notification system.
	This code has been simplified to assume that the requested user has no more pending responses to notifications.

Returns:
	0			this user has no more pending responses to notifications.
	1			this user has requested that their currently pending automatic order that has no available timeslots within their listed acceptable hours be cancelled for this orderInterval only
	integer format of date+requestedTimeslot+storeID		this user has requested that their currently pending automatic order that has no available timeslots within their listed acceptable hours be rescheduled for this date+timeslot
		ex. of above return: 20221205130001 - 2022 is the year, 12 is the month, 05 is the day, 13 is the index of the hour of the timeslot they request, and 0001 is the HEB storeID
*************************************/
int getNextUserNotificationResolution(int thisUserIDNum) {
	//check central HEB database of user notifications
	return 0;//simplified for this program to always return that there are no more tasks for this requested userIDNum
}

/****** isFreeTimeslot ***********
This function is code that already exists at HEB as part of the Curbside system.

Purpose:
	Returns whether the indicated timeslot is free enough to add another order to.
	This code is already implemented at HEB and has been simplified to always return that the timeslot is free.

Inputs:
	int storeIDNum				related storeID to this inquiry. Different stores have different availabilities.
	int orderDay				date of the requested inquiry
	int timeslotIndex			hour of the requested inquiry

Returns:
	1		timeslot is free enough
	2		timeslot is not free enough

*************************************/
int isFreeTimeslot(int storeID, int orderDay, int timeslotIndex) {
	return 1;//simplified to always work
}

/***** curbsideOrderPlacer ****
This function is code that already exists at HEB as part of the Curbside system.

Purpose:
	Places an order with HEB's Curbside system
	this function represents code that exists at HEB, and is simplified for use in this example

Parameters:
	int actionType					informs the curbsideOrderPlacer whether this is an order that needs to be cancelled or created. 1 is create order, 2 is cancel order.
	list<shoppingCartItem> cart		the shopping cart of the order to be either placed or cancelled
	userAccount thisUser			contains the information of the user for whom this order is being placed, and allows output to the user via notifications. All updates to notifications need to be sent to wherever user notifications are stored, which is not something stored within the scope of this program.

Returns:
	0		default error
	1		order placed successfully
	2		order cancelled successfully
	3		failed to place order due to requested timeslot being fully reserved already
	4		user not allowed to update this order
	5		failed to cancel this order
*******************************/
int curbsideOrderPlacer(int actionType, userAccount thisUser, int storeIDNum, int orderDay, int timeslotIndex, list<shoppingCartItem> cart ) {
	//this function represents the orderplacing system of HEB's Curbside System, which already exists at HEB and handles manual user input, but will accept automatic input from this program when it is implemented
	//this function will need to handle saving previous automatic orders, possibly separately from non-automatic orders, or at least marking them as having been automated for future metrics
	
	//for this example program, it is assumed that the curbside function works correctly every time, and places or cancels orders made with this automation program
	if (actionType == 1) {
		if (isFreeTimeslot(storeIDNum,orderDay,timeslotIndex)==1) {
			thisUser.notifications.push_front("Automatic order for " + to_string(orderDay) + " at " + to_string(timeslotIndex) + " ordered successfully.");
			//update user notification database here
			//updateUserNotificationDatabase(thisUser.userIDNum,thisUser.notifications.front(),"add");
			return 1;//order placed successfully
		} else {
			return 3;//timeslot already fully reserved
		}
	}
	else if (actionType == 2) {
		thisUser.notifications.push_front("Automatic order for " + to_string(orderDay) + " at " + to_string(timeslotIndex) + " cancelled successfully.");
		return 2;//order cancelled successfully
	}
}