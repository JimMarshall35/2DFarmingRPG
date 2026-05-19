#include <gtest/gtest.h>
#include "StringKeyHashMap.h"
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstdlib>
#include <cstdio>

bool gOverrideFuzzTestSeed = false;
unsigned int gFuzzTestSeed = 0;

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

using namespace std;

/*
	https://www.geeksforgeeks.org/dsa/convert-number-to-words/
	- actual correctness of implementation doesn't matter
*/
string convertToWords(int n)
{
    if (n == 0)
        return "Zero";

    // Words for numbers 0 to 19
    vector<string> units = {
        "",        "One",       "Two",      "Three",
        "Four",    "Five",      "Six",      "Seven",
        "Eight",   "Nine",      "Ten",      "Eleven",
        "Twelve",  "Thirteen",  "Fourteen", "Fifteen",
        "Sixteen", "Seventeen", "Eighteen", "Nineteen"
    };

    // Words for numbers multiple of 10        
    vector<string> tens = {
        "",     "",     "Twenty",  "Thirty", "Forty",
        "Fifty", "Sixty", "Seventy", "Eighty", "Ninety"
    };

    vector<string> multiplier =
    { "", "Thousand", "Million", "Billion" };

    string res = "";
    int group = 0;

    // Process number in group of 1000s
    while (n > 0) {
        if (n % 1000 != 0) {

            int value = n % 1000;
            string temp = "";

            // Handle 3 digit number
            if (value >= 100) {
                temp = units[value / 100] + " Hundred ";
                value %= 100;
            }

            // Handle 2 digit number
            if (value >= 20) {
                temp += tens[value / 10] + " ";
                value %= 10;
            }

            // Handle unit number
            if (value > 0) {
                temp += units[value] + " ";
            }

            // Add the multiplier according to the group
            temp += multiplier[group] + " ";

            // Add this group result to overall result
            res = temp + res;
        }
        n /= 1000;
        group++;
    }

    // Remove trailing space
    return res.substr(0, res.find_last_not_of(" ") + 1);
}



TEST(HashMap, InsertAndSearch)
{
	struct HashMap hashMap;
	HashmapInit(&hashMap, 10, sizeof(int));

	int* pEmpty = (int*)HashmapSearch(&hashMap, "FortyTwo");
	ASSERT_EQ(pEmpty, nullptr);

	int i = 42;
	HashmapInsert(&hashMap, "FortyTwo", &i);
	i = 64;
	HashmapInsert(&hashMap, "SixtyFour", &i);
	i = 180;
	HashmapInsert(&hashMap, "ONE HUNDRED AND EIGHTY!!", &i);
	
	int* pInt = (int*)HashmapSearch(&hashMap, "FortyTwo");
	ASSERT_NE(pInt, nullptr);
	ASSERT_EQ(*pInt, 42);

	pInt = (int*)HashmapSearch(&hashMap, "SixtyFour");
	ASSERT_NE(pInt, nullptr);
	ASSERT_EQ(*pInt, 64);
	
	pInt = (int*)HashmapSearch(&hashMap, "ONE HUNDRED AND EIGHTY!!");
	ASSERT_NE(pInt, nullptr);
	ASSERT_EQ(*pInt, 180);

	pInt = (int*)HashmapSearch(&hashMap, "three");
	ASSERT_EQ(pInt, nullptr);

	ASSERT_EQ(hashMap.size, 3);

	HashmapDeInit(&hashMap);
}

TEST(HashMap, InsertAndSearchStruct)
{
	struct TestStruct
	{
		int a;
		char b;
	};
	struct HashMap hashMap;
	HashmapInit(&hashMap, 10, sizeof(TestStruct));

	TestStruct i = {42, 'a'};
	HashmapInsert(&hashMap, "FortyTwo", &i);
	i = { 29, 'b' };
	HashmapInsert(&hashMap, "TwentyNine", &i); /* this tests an index collision as well, currently */
	i = { 8, 'c'};
	HashmapInsert(&hashMap, "Eight", &i);

	TestStruct* pInt = (TestStruct*)HashmapSearch(&hashMap, "FortyTwo");
	ASSERT_NE(pInt, nullptr);
	ASSERT_EQ(pInt->a, 42);
	ASSERT_EQ(pInt->b, 'a');

	pInt = (TestStruct*)HashmapSearch(&hashMap, "TwentyNine");
	ASSERT_NE(pInt, nullptr);
	ASSERT_EQ(pInt->a, 29);
	ASSERT_EQ(pInt->b, 'b');

	pInt = (TestStruct*)HashmapSearch(&hashMap, "Eight");
	ASSERT_NE(pInt, nullptr);
	ASSERT_EQ(pInt->a, 8);
	ASSERT_EQ(pInt->b, 'c');

	pInt = (TestStruct*)HashmapSearch(&hashMap, "fkoddfs");
	ASSERT_EQ(pInt, nullptr);

	HashmapDeInit(&hashMap);
}

/*
TODO: THIS SOMETIMES FAILS. Could be something to do with tests running in parallel

TEST(HashMap, Fuzz)
{
	struct HashMap hashMap;
	struct TestCase
	{
		int val;
		string key;
	};
	vector<TestCase> cases;
	HashmapInit(&hashMap, 10, sizeof(int));
	const int numInserts = 1024;
	const int numDeletes = 256;

	unsigned int seed = 0;
	if(gOverrideFuzzTestSeed)
	{
		seed = gFuzzTestSeed;
	}
	else
	{
		seed = (unsigned int)time(0);
	}
	
	srand(seed);
	printf("Seed: %u\n Repeat test with the cmd line flag:\n--hashmap-fuzz-seed %u\n", seed, seed);
	for (int i = 0; i < numInserts; i++)
	{
		int randomNum = rand() % 100000;
		string words = convertToWords(randomNum);
		if (HashmapInsert(&hashMap, const_cast<char*>(words.c_str()), &randomNum))
		{
			cases.push_back({ randomNum, words });
		}
	}

	for (int i = 0; i < numDeletes; i++)
	{
		int randomNum = rand() % cases.size();
		if (HashmapDeleteItem(&hashMap, const_cast<char*>(cases[randomNum].key.c_str())))
		{
			cases.erase(cases.begin() + randomNum);
		}
		else
		{
			printf("Delete failed, key '%s'\n", cases[randomNum].key.c_str());
			ASSERT_TRUE(false);
		}
	}
	ASSERT_EQ(cases.size(), hashMap.size);
	for (const TestCase& tc : cases)
	{
		int* pInt = (int*)HashmapSearch(&hashMap, const_cast<char*>(tc.key.c_str()));
		ASSERT_EQ(*pInt, tc.val) << tc.key;
	}
	HashmapDeInit(&hashMap);
}
*/

TEST(HashMap, HashmapKeyIterator)
{
	struct HashMap hashMap;
	HashmapInit(&hashMap, 10, sizeof(int));

	int i = 42;
	HashmapInsert(&hashMap, "FortyTwo", &i);
	i = 64;
	HashmapInsert(&hashMap, "SixtyFour", &i);
	i = 180;
	HashmapInsert(&hashMap, "ONE HUNDRED AND EIGHTY!!", &i);

	struct HashmapKeyIterator itr = GetKeyIterator(&hashMap);
	int on = 0;
	char* key = NextHashmapKey(&itr);
	while (key)
	{
		switch (on++)
		{
		case 0:
			ASSERT_EQ(strcmp(key, "FortyTwo"), 0);
			break;
		case 1:
			ASSERT_EQ(strcmp(key, "SixtyFour"), 0);
			break;
		case 2:
			ASSERT_EQ(strcmp(key, "ONE HUNDRED AND EIGHTY!!"), 0);
			break;
		default:
			ASSERT_TRUE(false);
			break;
		}
		key = NextHashmapKey(&itr);
	}

	ASSERT_TRUE(HashmapDeleteItem(&hashMap, "SixtyFour"));

	itr = GetKeyIterator(&hashMap);
	on = 0;
	key = NextHashmapKey(&itr);
	while (key)
	{
		switch (on++)
		{
		case 0:
			ASSERT_EQ(strcmp(key, "FortyTwo"), 0);
			break;
		case 1:
			ASSERT_EQ(strcmp(key, "ONE HUNDRED AND EIGHTY!!"), 0);
			break;
		default:
			ASSERT_TRUE(false);
			break;
		}
		key = NextHashmapKey(&itr);
	}

	HashmapDeInit(&hashMap);
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif