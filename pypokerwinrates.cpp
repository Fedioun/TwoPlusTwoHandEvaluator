#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <array>
#include <algorithm>
#include <unistd.h>
#include <cmath>
#include <string>
using namespace std;
#define DWORD int32_t


// The handranks lookup table- loaded from HANDRANKS.DAT.
int HR[32487834];
// The winrate before preflop lookup table- loaded from WinratesPreFlop.dat
float WR[1326];

float retrieve(int h0, int h1) {
  int a = min(h0, h1);
  int b = max(h0, h1);
  return WR[1326 - ((52-a)*(53-a))/2 + 52-b];
}

void UpdateSkipTable(array<int, 53> * skipTable, array<int, 7> * skippedValues, int nbSkippedValues) {
  int currentSkippedValue = 0;
  int offset = 0;
  bool good = true;
  for (int k = 1; k < 53; k++) {
    good = true;
    if (currentSkippedValue < nbSkippedValues) {
      if (k > (*skippedValues)[currentSkippedValue]){
        offset++;
        currentSkippedValue++;
      } else {
        if (k == (*skippedValues)[currentSkippedValue]) {
          good = false;
        }
      }
    }
    if (good) {
      (*skipTable)[k - offset] = k;
    }
    good = true;
  }
  for (int k = 0; k < offset; k++) {
    (*skipTable)[52 - k] = 0;
  }
}

float GetWinrateFlop(int h0, int h1, int c0, int c1, int c2) {
  int c3, c4; // Community cards

  // tables used to iterate over remaining cards in the deck
  std::array<int, 53> skipTable; // includes community cards
  std::array<int, 53> preFlopSkipTable; // only takes hole cards into account

  std::array<int, 7> skippedValues; // keeps track of picked cards
  std::array<int, 7> sortedSkippedValues; // same but sorted (a faster solution which doesn't need this exists)

  skippedValues[0] = h0;
  skippedValues[1] = h1;
  int u0, u1, u2, u3, u4, u5;

  int opponentValue = 0;
  int ownValue = 0;
  int communityValue = 0;
  long int wins = 0;
  long int games = 0;

  for (int k = 1; k < 53; k++){
    skipTable[k] = k;
    preFlopSkipTable[k] = k;
  }

  skippedValues[2] = c0;
  skippedValues[3] = c1;
  skippedValues[4] = c2;
  UpdateSkipTable(&preFlopSkipTable, &skippedValues, 5);
  u0 = HR[53+c0];
  u1 = HR[u0+c1];
  u2 = HR[u1+c2];

	for (c3 = 1; c3 < 47; c3++) {
  	u3 = HR[u2+preFlopSkipTable[c3]];
    skippedValues[5] = preFlopSkipTable[c3];
		for (c4 = c3+1; c4 < 48; c4++) {
      skippedValues[6] = preFlopSkipTable[c4];
      communityValue = HR[u3+preFlopSkipTable[c4]];
      ownValue = HR[communityValue + h0];
      ownValue = HR[ownValue + h1];
      sortedSkippedValues = skippedValues;
      std::sort(sortedSkippedValues.begin(), sortedSkippedValues.end());
      UpdateSkipTable(&skipTable, &sortedSkippedValues, 7);
      for (int o0 = 1; o0 < 45; o0++) {
        for (int o1 = o0 +1 ; o1 < 46; o1++) {
          games++;
          opponentValue = HR[communityValue + skipTable[o0]];
          opponentValue = HR[opponentValue + skipTable[o1]];
          if ((ownValue >> 12) > (opponentValue >> 12)) {
            wins++;
          } else {
            if ((ownValue >> 12) == (opponentValue >> 12)) {
              if ((ownValue & 0x00000FFF) > (opponentValue & 0x00000FFF)) {
                wins++;
              }
            }
          }
    		}
    	}
    }
  }
  //cout << "Community : " << c0 << " " << c1 << " " << c2 << endl;
  //cout << "wins : " << wins << "  games : " << games << " Ratio : " << ((long double)(wins) / (long double)(games)) << endl;
  return ((long double)(wins) / (long double)(games));
}

float GetWinrateRiver(int h0, int h1, int c0, int c1, int c2, int c3) {
  int c4; // Community cards

  // tables used to iterate over remaining cards in the deck
  std::array<int, 53> skipTable; // includes community cards
  std::array<int, 53> preFlopSkipTable; // only takes hole cards into account

  std::array<int, 7> skippedValues; // keeps track of picked cards
  std::array<int, 7> sortedSkippedValues; // same but sorted (a faster solution which doesn't need this exists)

  skippedValues[0] = h0;
  skippedValues[1] = h1;
  int u0, u1, u2, u3, u4, u5;

  int opponentValue = 0;
  int ownValue = 0;
  int communityValue = 0;
  long int wins = 0;
  long int games = 0;

  for (int k = 1; k < 53; k++){
    skipTable[k] = k;
    preFlopSkipTable[k] = k;
  }

  skippedValues[2] = c0;
  skippedValues[3] = c1;
  skippedValues[4] = c2;
  skippedValues[5] = c3;
  UpdateSkipTable(&preFlopSkipTable, &skippedValues, 6);
  u0 = HR[53+c0];
  u1 = HR[u0+c1];
  u2 = HR[u1+c2];
  u3 = HR[u2+c3];
	for (c4 = 1; c4 < 47; c4++) {


    skippedValues[6] = preFlopSkipTable[c4];
    communityValue = HR[u3+preFlopSkipTable[c4]];
    ownValue = HR[communityValue + h0];
    ownValue = HR[ownValue + h1];
    sortedSkippedValues = skippedValues;
    std::sort(sortedSkippedValues.begin(), sortedSkippedValues.end());
    UpdateSkipTable(&skipTable, &sortedSkippedValues, 7);
    for (int o0 = 1; o0 < 45; o0++) {
      for (int o1 = o0 +1 ; o1 < 46; o1++) {
        /**
        if (games% 1000 == 0){
          cout << endl;
          cout << "Own value " << (ownValue >> 12)<< " " << (ownValue & 0x00000FFF) << endl;
          usleep(10000);
          cout << endl;
          for (int j = 1; j < 53; j++) {

            cout << skipTable[j] << " ";

          }
          cout << endl;
          for (int j = 0; j < 7; j++) {
            cout << skippedValues[j] << " ";
          }
          cout << endl << skipTable[o0] << "  " << skipTable[o1] << "   " << skipTable[45]<< endl;

        }**/


        games++;
        opponentValue = HR[communityValue + skipTable[o0]];
        opponentValue = HR[opponentValue + skipTable[o1]];
        if ((ownValue >> 12) > (opponentValue >> 12)) {
          wins++;
        } else {
          if ((ownValue >> 12) == (opponentValue >> 12)) {
            if ((ownValue & 0x00000FFF) > (opponentValue & 0x00000FFF)) {
              wins++;
            }
          }
        }
  		}
  	}
  }
  //cout << "Community : " << c0 << " " << c1 << " " << c2 << endl;
  //cout << "wins : " << wins << "  games : " << games << " Ratio : " << ((long double)(wins) / (long double)(games)) << endl;
  return ((long double)(wins) / (long double)(games));
}

float GetWinrateTurn(int h0, int h1, int c0, int c1, int c2, int c3, int c4) {
  // tables used to iterate over remaining cards in the deck
  std::array<int, 53> skipTable; // includes community cards
  std::array<int, 7> skippedValues; // keeps track of picked cards
  std::array<int, 7> sortedSkippedValues; // same but sorted (a faster solution which doesn't need this exists)

  skippedValues[0] = h0;
  skippedValues[1] = h1;
  int u0, u1, u2, u3, u4, u5;

  int opponentValue = 0;
  int ownValue = 0;
  int communityValue = 0;
  long int wins = 0;
  long int games = 0;

  for (int k = 1; k < 53; k++){
    skipTable[k] = k;
  }

  skippedValues[2] = c0;
  skippedValues[3] = c1;
  skippedValues[4] = c2;
  skippedValues[5] = c3;
  skippedValues[6] = c4;

  u0 = HR[53+c0];
  u1 = HR[u0+c1];
  u2 = HR[u1+c2];
  u3 = HR[u2+c3];
  communityValue = HR[u3+c4];

  ownValue = HR[communityValue + h0];
  ownValue = HR[ownValue + h1];

  sortedSkippedValues = skippedValues;
  std::sort(sortedSkippedValues.begin(), sortedSkippedValues.end());
  UpdateSkipTable(&skipTable, &sortedSkippedValues, 7);

  for (int o0 = 1; o0 < 45; o0++) {
    for (int o1 = o0 +1 ; o1 < 46; o1++) {
      games++;
      opponentValue = HR[communityValue + skipTable[o0]];
      opponentValue = HR[opponentValue + skipTable[o1]];
      if ((ownValue >> 12) > (opponentValue >> 12)) {
        wins++;
      } else {
        if ((ownValue >> 12) == (opponentValue >> 12)) {
          if ((ownValue & 0x00000FFF) > (opponentValue & 0x00000FFF)) {
            wins++;
          }
        }
      }
    }
  }
  //cout << "Community : " << c0 << " " << c1 << " " << c2 << endl;
  //cout << "wins : " << wins << "  games : " << games << " Ratio : " << ((long double)(wins) / (long double)(games)) << endl;
  return ((long double)(wins) / (long double)(games));
}

/**
  h0, h1 : hole cards
  nb_players : number of players, including you
*/
float GetWinratePreFlop(int h0, int h1){
  return retrieve(h0, h1);
}

void initialization() {
  printf("Loading HandRanks.DAT file... \n");
	memset(HR, 0, sizeof(HR));
	FILE * fin = fopen("HandRanks.dat", "rb");
	if (!fin){
    printf("Hum hum... Please generate HandRanks.DAT using \"make\"\n");
  }
	size_t bytesread = fread(HR, sizeof(HR), 1, fin);	// get the HandRank Array

  printf("Loading WinratesPreFlop.DAT file... \n");
  memset(WR, 0, sizeof(WR));
	FILE * fwin = fopen("WinratesPreFlop.dat", "rb");
	if (!fwin){
    printf("Hum hum... Please generate WinratesPreFlop.dat by calling generateWinratePreFlopTable() \n");
  }
	size_t bytesreadwr = fread(WR, sizeof(WR), 1, fwin);	// get the HandRank Array

	fclose(fwin);
	printf("complete.\n");
}

int main() {
	initialization();
  cout << GetWinrateFlop(2, 50, 4, 52, 51) << endl;
  cout << GetWinratePreFlop(2, 50) << endl;
  cout << GetWinrateRiver(12, 25, 18, 7, 15, 50) << endl;
  cout << GetWinrateTurn(2, 50, 4, 52, 51, 42, 22) << endl;
	return 0;
}


#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(pypokerwinrates)
{
    def("initialization", initialization);
    def("GetWinratePreFlop", GetWinratePreFlop);
    def("GetWinrateFlop", GetWinrateFlop);
    def("GetWinrateRiver", GetWinrateRiver);
    def("GetWinrateTurn", GetWinrateTurn);
}
