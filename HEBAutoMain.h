

//TODO:check if includes are required in a .h file

#include <list>
#include <string>

#define SHOPPINGCARTITEMLIMIT 256

using namespace std;

struct userAccount {
	int userIDNum;//unique account identification number; generated at account creation
	bool notificationAllowedTimes[24];//an array of hours every day the user is allowed to receive notifications; this is set up by the user at account creation
	list<string> notifications;//a list of notifications that are awaiting user response/input

	//the following variable should actually be something handled by the curbside app
	//bool notificationAllowedTypes[];//an array that the user sets upon signup, which controls phone/email notification types
	
} ;

struct shoppingCartItem {
	int itemID;//grocery item id number
	int unitQuantity;//quantity of this item in the shopping cart
	//float unitPrice;//price of this item (not relevant to this order-placing system)
};

void main();
void midnightScheduler();
void signUps();
int timeslotChecker(bool theseTimeslots[24], int userIDNum, int storeIDNum, int orderDay);
void userNotifications();
void updateUserNotificationDatabase(int thisUserIDNum, string thisNotification, string actionType);
int isFreeTimeslot(int storeID, int orderDay, int timeslotIndex);
int curbsideOrderPlacer(int actionType, userAccount thisUser, int storeIDNum, int orderDay, int timeslotIndex, list<shoppingCartItem> cart);
