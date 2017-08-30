#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include <time.h>
#include <sys/time.h>

using namespace std;

int deck[52];
int count = 0;

class state
{
	public:
		int reward[3];//[action][reward gained during that action];
		int policy;
		
		state()
		{
			policy = -1;
			for(int i = 0; i< 3;i++)
				reward[i] = 0;
			
		}
};

class hand
{
	public:
		int cards[10];
		int has_ace;
		int value;
		
	hand()
	{
		for (int i =0; i < 10; i ++) 
			cards[i] = -1;
		has_ace = 0;
		value = 0;
	}
	
	int reset()
	{
		for (int i =0; i < 10; i ++) 
			cards[i] = -1;
		has_ace = 0;
		value = 0;	
	}
		
};

class agent
{
	public:
		hand my_hand,de_hand;
		int bet;
		int busted;
		int action;
		int gameover;
		int gamestart;
		
		agent()
		{
			bet = 10;
			busted = 0;
			gameover =0;
			gamestart =0;
			action = rand() % 3;
		}
		
		int reset()
		{
			bet = 10;
			busted = 0;
			gameover =0;
			action = rand() % 3;
			my_hand.reset();
			de_hand.reset();
			
		}
		
};

unsigned long long rank;
int AgentDebug =0,InitializeDeckDebug = 0, DealCardDebug = 0,PlayGameDebug = 0,UpdateStateDebug = 0;
agent a1;
state* s1;

void InitializeDeck();
char CardName(int i);
void DealCard(hand* this_hand);
int HandValue(hand* this_hand);
int CardValue(hand* this_hand,int i);
int GetState();
int PlayGame(int Action);
int UpdateState(int result);

void InitializeDeck()
{
	int k = 0;
	for(int i = 0; i< 52; i++)
	{
		if(i% 13 == 0) k = 0;
		deck[i] = k+1;
		k++;	
	}
	random_shuffle ( &deck[0], &deck[51] );
	if (InitializeDeckDebug == 1) for(int i = 0; i< 52; i++) cout << "Value at Deck "<< i << ": " <<deck[i] << endl;
}

char CardName(int i)
{
	if(i == 1)
		return 'A';
	else if(i> 1 && i < 10)
	{	
		char a = '0' + i;
		return a;
	}
	else
		return 'F';
}

void DealCard(hand* this_hand)
{
	int drawn = 1;
	int drawn_card = -1;
	while(drawn == 1)
	{
		drawn_card = rand() % 52;
		
		if (deck[drawn_card] != 0) 
		{
			drawn = 0;
			for (int i = 0; i < 10; i++)
			{
				if(this_hand->cards[i] == -1) 
				{
					this_hand->cards[i] = deck[drawn_card]; 
					if(deck[drawn_card] == 1) this_hand->has_ace = 1;
					if(DealCardDebug == 1)
					{	
						if(this_hand == &a1.de_hand) cout << "\nDealer"; 
						else cout << "\nPlayer"; 
						cout << " Card Drawn is :" <<  CardName(this_hand->cards[i]);
					}
					break;
				}
			}//for		
			deck[drawn_card] = 0;
			drawn = 0;
		}//if		
	}//while
	HandValue(this_hand);
	//cout << endl;
	return;
}

int HandValue(hand* this_hand)
{
	this_hand->value = 0;
	for(int i = 0; i < 10; i++)
	{
		if(this_hand->cards[i] > 1 && this_hand->cards[i] < 10)
		{
			this_hand->value += this_hand->cards[i];
		}
		else if(this_hand->cards[i] > 9)
		{	
			this_hand->value += 10;
		}
		else if (this_hand->cards[i] == 1)
		{
			this_hand->value += 11;
		}
	}
	
	for(int i = 0; i < 10; i++)
	{
		if(this_hand->value > 21)
		{
			if(this_hand->cards[i] == 1)
				this_hand->value -= 10;
		}
		else
			break;	
	}	
//cout << "\nValue of Hand is : \t" << this_hand->value;
//cout << endl;
	return this_hand->value;
}

int CardValue(hand* this_hand,int i)
{
	if(this_hand->cards[i] > 1 && this_hand->cards[i] < 10)
	{
		return this_hand->cards[i];
	}
	else if(this_hand->cards[i] > 9)
	{	
		return 10;
	}
	else if (this_hand->cards[i] == 1)
	{
		return 1;
	}
	
//cout << "\nValue of Hand is : \t" << this_hand->value;
//cout << endl;
	return this_hand->value;
}

int GetState()
{
	int DealerCard = CardValue(&a1.de_hand,0);
//cout << "\n\nGetState Start. \n\nDealer Has: " << DealerCard;
	int PlayerValue = 0;
	int no_of_aces = 0;
	
	for(int i = 0; i < 2; i++)
	{
		if(a1.my_hand.cards[i] > 1 && a1.my_hand.cards[i] < 10)
		{
			PlayerValue += a1.my_hand.cards[i];
		}
		
		else if (a1.my_hand.cards[i] == 1)
		{
			PlayerValue += 19;
			no_of_aces++;
		}
		else 
		{	
			PlayerValue += 10;
		}
	}
	
	if(no_of_aces == 2)
	{
//cout << "\n\nState Row is " << 30 << "\tColumn is " << DealerCard << endl;
		return (30*10)+DealerCard;
	}
	else
	{
//cout << "\n\nState Row is " << PlayerValue << "\tColumn is " << DealerCard<< endl;
		return (PlayerValue*10)+DealerCard; 
	}
	/*else if(no_of_aces == 1)
	{
		cout << "\n\n Row is " << PlayerValue << "\tColumn is " << DealerCard<< endl;
		return PlayerValue*10+DealerCard; 
	}*/ 

}

int PlayGame(int Action)
{
	
	if(a1.gamestart == 0) 
	{
		a1.action = Action;
		a1.gamestart = 1;
//cout << "\n\t\t\t\t\ta1[" << rank << "].action = " << a1.action;
	}

	if(HandValue(&a1.my_hand) > 21) 
	{
		if(PlayGameDebug == 1) cout << "\nPlayer Busted!";
		a1.busted = 1;
		a1.gameover = 1;
		UpdateState(-1);
		return 0;
	}
	
	if(Action == 0 || a1.bet == 20) // STAND
	{
		if(PlayGameDebug == 1) cout << "\nPlayer Stood!";
		
		while(HandValue(&a1.de_hand) < 17)
			DealCard(&a1.de_hand);
		if(HandValue(&a1.de_hand) > 21)
		{
			if(PlayGameDebug == 1) cout << "\nDealer Busted!";
			UpdateState(1);
		}
		else if(HandValue(&a1.de_hand) == HandValue(&a1.my_hand))
		{	
			if(PlayGameDebug == 1) cout << "\nDraw!";
			UpdateState(0);
		}
		else
		{	
			if(PlayGameDebug == 1) cout << "\nPlayer Loses.";
			UpdateState(-1);
		}
		a1.gameover = 1;
		return 0;
	}
	else if(Action == 1) //HIT
	{	
	
//cout << "\nBet: "<< a1.bet;
		if(a1.bet == 20) return 0;
		
		if(PlayGameDebug == 1) cout << "\nPlayer Hit!";
		DealCard(&a1.my_hand);
		
	}
	else if(Action == 2) // DOUBLE
	{
		if(a1.my_hand.cards[2] != -1) return 0;
		
		if(PlayGameDebug == 1) cout << "\nPlayer Doubled!";

		DealCard(&a1.my_hand);
		a1.bet = 20;
	}
}

int UpdateState(int result)
{
	//cout << "\nUpdate State Start \tAction : " << a1.action;
	
	
	if(a1.action == -1) return 0;
	int check = 0;
	//cout << "\nAction : " << a1.action;
	
	{
		int selected_state = GetState(),max = -9999;
		s1[selected_state].reward[a1.action] += (a1.bet * result);
		for(int i = 0; i < 3;i++)
		{
			if(s1[selected_state].reward[a1.action] > max)
			{
				s1[selected_state].policy = a1.action;
				max = s1[selected_state].reward[a1.action];
				check++;
			}
			
		}
		
		if (UpdateStateDebug == 1)
			cout << "\nState :" << selected_state << "\tReward : "<< result * a1.bet << "\tAction : " << a1.action << "\tPolicy : " << s1[selected_state].policy << "\t Check: " << check;
		 
	}
	
	//cout << "\nUpdate State End " ;
	
	return 0;
	
}

double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cerr << "Usage: \n";
		exit(1);
	}
	unsigned long long no_of_agents = atoll(argv[1]) ;
	int thread_count = atoi(argv[2]);
	
	cout << "\n******************************Reinforcement Learning******************************************\n\n";	
	
	srand(time (NULL));
	
	s1 = new state[(310)];
	
	double wall0 = get_wall_time();

	for(unsigned long long ag1 =0; ag1 < no_of_agents; ag1++)
	{	
		{
			if(AgentDebug == 1) cout << "\n\n\n**********************************New AGENT*********************************************\n\n\n";
			if(AgentDebug == 1) cout << "\nIteration : "<< ag1;
		}
		
		if (ag1 % 1000000 == 0) cout << "\nReached Agent no. : "<<ag1;
		
		DealCardDebug = 0;
		AgentDebug = 0;
		PlayGameDebug = 0;
		UpdateStateDebug = 0;
		
		InitializeDeck();
		
		DealCard(&a1.my_hand);	
		DealCard(&a1.my_hand);
		if(AgentDebug == 1) cout << "\nValue of My Hand is : " << HandValue(&a1.my_hand);
		
		DealCard(&a1.de_hand);	
		DealCard(&a1.de_hand);
		
		if(AgentDebug == 1) cout << "\nDealer's Visible Card is : " << CardName(a1.de_hand.cards[0]);

		//GetState();
		
		while (a1.gameover == 0)
		{	
			int action;
			int decision = rand % 4;
			if(decision < 3)
				action = rand() % 3;
			else
				action = s1[GetState()].policy;
			
			
			if(a1.bet == 20) action = 0;	

			PlayGame(action);
			if(a1.gameover == 1) 
			{
				a1.reset();
				break;
			}
		}//while 
	}//parallel for

	double wall1 = get_wall_time();

	for(int i = 0; i < 310; i++)
	{
		if(i%10 == 0) cout <<endl<<i/10 << endl;
		cout << "\t "<< i % 10 <<" : " << s1[i].policy; 
	}
	
    cout << "\n\n\n\nWall Time = " << wall1 - wall0 << "s"<<endl;

	cout << "\n\n" <<endl;
		
	return 0;

}

/*
for(int i = 0; i < 10; i++)
	{
		cout << " \n a1.my_hand.cards["<< i <<" ] : " << a1.my_hand.cards[i]; 
	}
	
	for(int i = 0; i < 10; i++)
	{
		cout << " \n a1.my_hand.cards["<< i <<" ] : " << a1.de_hand.cards[i]; 
	}
	
*/
