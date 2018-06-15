#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <string>
#include "lpclib.h"
#include "vector.h"
#include "scanner.h"
#include "lexicon.h"
#include "hgraphics.h"
using namespace std;

struct pointOnBoard
{
    int x;
    int y;
    pointOnBoard(int xx=0, int yy=0): x(xx), y(yy) { }
};

Lexicon lex("lexicon.dat");

Vector<string> HumanTurn();
void CompTurn(Vector <string> userScores);
bool formableWord(int r, int c, string soFar, string orig, Vector<pointOnBoard> & wordPath);
bool createWord(int r, int c, string & word, string soFar, Vector<string> wordBag, Vector<pointOnBoard> & wordPath);
bool alreadyMade(int r, int c, Vector <pointOnBoard> wordPath);
bool usedWord(Vector<string> wordBag, string word);
bool inBounds(int r, int c);
void showPathHuman(Vector<pointOnBoard>wordPath);///the users wordPath, slow speed for visual
void showPathComputer(Vector<pointOnBoard>wordPath);///fast speed for computer word finder
 
int main()
{
    srand(time(NULL));
    ///new file for dice
    ifstream fin;

    string line;
    fin.open("dice.txt");
    vector<string> strings;
    for (int x = 0; x < 19; x++)
    {
        fin >> line;
        strings.push_back(line);
    }
    cout << "Click the mouse after the computer's turn to end the program. Thank you!\n";
    fin.close();
    DrawBoard();

    for (int r = 0; r < 5; r++)
    {
        for (int c = 0; c < DiceInRow(r); c++)
        {
            string s = strings[rand()%19];
            char die = s[rand()%6];
            LabelHex(r, c, die);
        }
    }
    UpdateDisplay();
    PrintMessage("Welcome to the wonderful game of Hoggle!\n");
    PrintMessage ("Enter your name-> ");
    string name = BoardGetLine();
    PrintMessage (string("Good luck, " + name + ", you'll need it.\n"));

    for (int r = 0; r < 5; r++)
    {
        for (int c = 0; c < DiceInRow(r); c++)
        {
            HighlightHex(r, c, true);
            Pause(0.1);
            HighlightHex(r, c, false);
        }
    }
    Vector<string> userC = HumanTurn();
    CompTurn(userC);
    WaitForMouseDown();///to keep board open until mouse down
	return 0;
}

bool usedWord(Vector<string> wordVector, string word)
{
    for (int i =0; i < wordVector.size(); i++)
    {
        if (word == wordVector.getAt(i))
        {
            return true;
        }
    }
    return false;
}

Vector<string> HumanTurn()
{
    string word = "start";
    Vector<string> temp;
    if (word =="")
    {
          PrintMessage("Computer's turn.\n");
    }
    while(word!= "")
    {
        bool scored = true;
        PrintMessage ("Enter a word: ");
        word = BoardGetLine();

        string upWord = ConvertToUpperCase(word);

        if (upWord.length() <3 && upWord != "")
        {
            PrintMessage("The word must be at least 3 letters long.\n");
        }
        else if (usedWord(temp,upWord))
        {
            PrintMessage("That word was already played.\n");
        }
        else
        {
            for (int r = 0; r < 5; r++)
            {
                scored = false;
                for (int c = 0; c < DiceInRow(r); c++)
                {

                    if (LetterAt(r,c) == upWord[0])
                    {
                        Vector<pointOnBoard> highLightPos;
                        highLightPos.add(pointOnBoard(r,c));
                        bool wordCanBeFormed = formableWord(r,c,upWord.substr(0,1),upWord, highLightPos);
                        if (wordCanBeFormed && lex.containsWord(upWord))
                        {
                            temp.add(upWord);
                            scored = true;

                            RecordNewWord(upWord, Human);

                            for (int length = 3; length <= 7; length++)
                            {
                                if (word.length()==length)
                                {
                                    string pt = IntToString(length-2);
                                    PrintMessage(pt + " point(s) awarded.\n");
                                }
                            }
                            showPathHuman(highLightPos);
                            break;
                        }
                        else if(wordCanBeFormed && !lex.containsWord(upWord))
                        {
                            scored = true;
                            PrintMessage("That word is on the board, but is not in the lexicon.\n");
                            showPathHuman(highLightPos);
                            break;
                        }
                        highLightPos.clear();
                    }
                    if (scored)
                        break;
                }
                if (scored)
                    break;
            }
            if(!scored)
            {
                PrintMessage("That word is not on the board.\n");
            }
        }
    }
    return temp;
}

void CompTurn(Vector <string> userScores)
{
    string word;
    bool wordFound;
    Vector<pointOnBoard> AIwordPath;

    for (int r = 0; r < 5; r++)
    {
        for (int c = 0; c < DiceInRow(r); c++)
        {
            word = string(1, LetterAt(r,c));

            AIwordPath.add(pointOnBoard(r,c));
            string output = "";
            wordFound = createWord(r,c,output, word, userScores,AIwordPath);

            while (wordFound)
            {
                userScores.add(output);
                PrintMessage("Computer has found a word: " + output + "\n");
                for (int length = 3; length <= 7; length++)
                {
                    if (output.length()==length)
                    {
                        string pt = IntToString(length-2);
                        PrintMessage(pt + " point(s) awarded\n");
                    }
                }
                showPathComputer(AIwordPath);
                RecordNewWord(output,Computer);
                AIwordPath.clear();
                AIwordPath.add(pointOnBoard(r,c));

                output = "";
                wordFound = createWord(r,c, output, word, userScores,AIwordPath);
            }
            AIwordPath.clear();
        }
    }
}

bool formableWord(int r, int c,  string soFar, string orig, Vector<pointOnBoard> & wordPath)
{
    static int topX[6] = {-1,-1,0,1,1,0};
    static int topY[6] = {-1,0,1,1,0,-1};
    static int midX[6] = {-1,-1,0,1,1,0};
    static int midY[6] = {-1,0,1,0,-1,-1};
    static int botX[6] ={-1,-1,0,1,1,0};
    static int botY[6] = {0,1,1,0,-1,-1};

    int newR;
    int newC;

    if (soFar == orig)
    {
        return true;
    }
    else
    {
        for (int a =0; a < 6; a++)
        {
            if (r == 0 || r == 1)
            {
                newR = r + topX[a];
                newC = c + topY[a];
            }
            else if (r ==2)
            {
                newR = r +midX[a];
                newC = c +midY[a];
            }
            else
            {
                newR = r + botX[a];
                newC = c + botY[a];
            }
            if (inBounds(newR,newC) && !alreadyMade(newR,newC,wordPath))
            {
                if (LetterAt(newR,newC) == orig[soFar.size()])
                {
                    wordPath.add(pointOnBoard(newR,newC));
                    if (formableWord(newR,newC, soFar + LetterAt(newR,newC), orig,wordPath))
                    {
                        return true;
                    }
                    else
                    {
                        wordPath.removeAt(wordPath.size()-1);
                    }
                }
            }
        }
    }
    return false;
}
bool createWord(int r, int c, string &wordReturn,string soFar, Vector<string> wordBag, Vector<pointOnBoard> & wordPath)
{
    static int topX[6] = {-1,-1,0,1,1,0};
    static int topY[6] = {-1,0,1,1,0,-1};
    static int midX[6] = {-1,-1,0,1,1,0};
    static int midY[6] = {-1,0,1,0,-1,-1};
    static int botX[6] ={-1,-1,0,1,1,0};
    static int botY[6] = {0,1,1,0,-1,-1};

    int newR;
    int newC;
    if (lex.containsWord(soFar) && soFar.length() >3 && !usedWord(wordBag,soFar))
    {
        wordReturn=soFar;
        return true;
    }

    else if (!lex.containsPrefix(soFar))
    {
        return false;
    }
    else
    {
        for (int d =0; d < 6; d++)
        {
            if (r == 0 || r == 1)
            {
                newR = r + topX[d];
                newC = c + topY[d];
            }
            else if (r ==2)
            {
                newR = r +midX[d];
                newC = c +midY[d];
            }
            else
            {
                newR = r + botX[d];
                newC = c + botY[d];
            }

            if (inBounds(newR,newC) && !alreadyMade(newR,newC,wordPath))
            {
                wordPath.add(pointOnBoard(newR,newC));
                if (createWord(newR,newC,wordReturn,soFar+LetterAt(newR,newC),wordBag,wordPath))
                {
                    return true;
                }
                else
                {
                    wordPath.removeAt(wordPath.size()-1);
                }
            }
        }
    }
    return false;
}

bool alreadyMade(int r, int c, Vector<pointOnBoard> wordPath)
{
    for (int b =0; b < wordPath.size(); b++)
    {
        if (c == wordPath.getAt(b).x && c ==  wordPath.getAt(b).y)
        {
            return true;
        }
    }
    return false;
}

bool inBounds(int r, int c)
{
    if (r < 0 || r >4)
    {
        return false;
    }
    else if ((r ==0 || r == 4) && (c < 0 || c >2))
    {
        return false;
    }
    else if ((r ==1 || r == 3) && (c < 0 || c >3))
    {
        return false;
    }
    else if ((r ==2) && (c < 0 || c >4))
    {
        return false;
    }
    else
        return true;
}

void showPathHuman(Vector<pointOnBoard>wordPath)
{
    for (int c =0; c < wordPath.size(); c++)
    {
        HighlightHex(wordPath.getAt(c).x  , wordPath.getAt(c).y, true);

        Pause(.1);
        HighlightHex(wordPath.getAt(c).x  , wordPath.getAt(c).y, false);
    }
}

void showPathComputer(Vector<pointOnBoard>wordPath)
{
    for (int c =0; c < wordPath.size(); c++)
    {
        HighlightHex(wordPath.getAt(c).x  , wordPath.getAt(c).y, true);
        Pause(.05);
        HighlightHex(wordPath.getAt(c).x  , wordPath.getAt(c).y, false);
    }
}

