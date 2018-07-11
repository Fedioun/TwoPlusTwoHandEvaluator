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

void store(int h0, int h1, float wr) {
  int a = min(h0, h1);
  int b = max(h0, h1);
  WR[1326 - ((52-a)*(53-a))/2 + 52-b] = wr;
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

/**
  h0, h1 : hole cards
  nb_players : number of players, including you
*/
float GetWinrate(int h0, int h1){

  int c0, c1, c2, c3, c4; // Community cards

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

  UpdateSkipTable(&skipTable, &skippedValues, 2);
  UpdateSkipTable(&preFlopSkipTable, &skippedValues, 2);

  for (c0 = 1; c0 < 47 ; c0++) {
  	u0 = HR[53+preFlopSkipTable[c0]];
    skippedValues[2] = preFlopSkipTable[c0];

  	for (c1 = c0+1; c1 < 48; c1++) {
  		u1 = HR[u0+preFlopSkipTable[c1]];
      skippedValues[3] = preFlopSkipTable[c1];

  			for (c2 = c1+1; c2 < 49; c2++) {
  			u2 = HR[u1+preFlopSkipTable[c2]];
        skippedValues[4] = preFlopSkipTable[c2];

    			for (c3 = c2+1; c3 < 50; c3++) {
  				u3 = HR[u2+preFlopSkipTable[c3]];
          skippedValues[5] = preFlopSkipTable[c3];

  					for (c4 = c3+1; c4 < 51; c4++) {
            skippedValues[6] = preFlopSkipTable[c4];

            int communityValue = HR[u3+preFlopSkipTable[c4]];
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
  		}
  	}
  }

  /////////////////////////////////////////////////
  return ((long double)(wins) / (long double)(games));
}



void generateWinratePreFlopTable() {
  string corres[4] = {"Jack", "Queen", "King", "Ace"};
  float p, ps, pd;
  cout << endl << "Precomputing winrates for every hand at preflop :" << endl << "(it may take a long time)" << endl;
  for(int i = 0; i < 13; i++) {
    for (int j = i; j < 13; j++) {
      if (i == j) {
        if (i > 8) {
          cout << endl << "Pair of " << corres[i - 9] << "..." << endl;
        } else {
          cout << endl << "Pair of " << i+2 << " ..." << endl;
        }
        p = GetWinrate(i*4+1, j*4+2);
        cout << " winrate : " << p << "\%" << endl;

        store(4*i+1, 4*j+2, p);
        store(4*i+1, 4*j+3, p);
        store(4*i+1, 4*j+4, p);
        store(4*i+2, 4*j+3, p);
        store(4*i+2, 4*j+4, p);
        store(4*i+3, 4*j+4, p);
      } else {
        cout << endl;
        if (i > 8) {
          cout << corres[i - 9] << " and ";
        } else {
          cout << i + 2 << " and ";
        }
        if (j > 8 ) {
          cout << corres[j - 9] << " of the same suit..." << endl;
        } else {
          cout << j + 2  << " of the same suit..." << endl;
        }
        ps = GetWinrate(i*4+1, j*4+1);
        cout << " winrate : " << ps << "\%" << endl;
        store(4*i+1, 4*j+1, ps);
        store(4*i+2, 4*j+2, ps);
        store(4*i+3, 4*j+3, ps);
        store(4*i+4, 4*j+4, ps);

        cout << endl;
        if (i > 8) {
          cout << corres[i - 9] << " and ";
        } else {
          cout << i + 2 << " and ";
        }
        if (j > 8 ) {
          cout << corres[j - 9] << " of different suits..." << endl;
        } else {
          cout << j + 2 << " of different suits..." << endl;
        }
        pd = GetWinrate(i*4+1, j*4+2);
        cout << " winrate : " << pd << "\%" << endl;

        store(4*i+1, 4*j+2, pd);
        store(4*i+1, 4*j+3, pd);
        store(4*i+1, 4*j+4, pd);

        store(4*i+2, 4*j+1, pd);
        store(4*i+2, 4*j+3, pd);
        store(4*i+2, 4*j+4, pd);

        store(4*i+3, 4*j+1, pd);
        store(4*i+3, 4*j+2, pd);
        store(4*i+3, 4*j+4, pd);

        store(4*i+4, 4*j+1, pd);
        store(4*i+4, 4*j+2, pd);
        store(4*i+4, 4*j+3, pd);

      }
    }
  }


  FILE * fout = fopen("WinratesPreFlop.dat", "wb");
  if (!fout) {
    printf("Problem creating the Output File!\n");
  }
  fwrite(WR, sizeof(WR), 1, fout);  // big write, but quick

  fclose(fout);
  printf("Succes : WinratesPreFlop.dat generated.\n");
}


int main() {
  memset(WR, 0, sizeof(WR));

	printf("Generating preflop winrate table\n");
	printf("-----------------------------------------\n\n");

	// Load the HandRanks.DAT file and map it into the HR array
	printf("Loading HandRanks.DAT file... \n");
	memset(HR, 0, sizeof(HR));
	FILE * fin = fopen("HandRanks.dat", "rb");
	if (!fin){
    printf("Hum hum... Please generate HandRanks.DAT using \"make\"\n");
    return false;
  }
	size_t bytesread = fread(HR, sizeof(HR), 1, fin);	// get the HandRank Array

	printf("complete.\n");

  generateWinratePreFlopTable();
	return 0;
}
