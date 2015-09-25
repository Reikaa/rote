#pragma once
#ifndef TMEN_TESTS_RSGREDIST_HPP
#define TMEN_TESTS_RSGREDIST_HPP

#include "tensormental/tests/AllRedists.hpp"
using namespace tmen;

void
CreateRSGTestsSinkHelper(const ModeArray& modesToMove, const ModeArray& sinkModesGroup, const ModeArray& reduceModes, const TensorDistribution& distA, const std::vector<RedistTest>& partialTests, std::vector<RedistTest>& fullTests){
	Unsigned order = distA.size() - 1;
	Unsigned i, j;

	if(modesToMove.size() == 0){
//		printf("adding fullTest\n");
//		for(i = 0; i < partialTests.size(); i++){
//			PrintVector(partialTests[i].second, "commModes");
//			std::cout << TensorDistToString(partialTests[i].first) << std::endl;
//		}
//		printf("done\n");
		for(i = 0; i < partialTests.size(); i++){
			RedistTest partialTest = partialTests[i];
			TensorDistribution resDist = partialTest.first;

			ModeArray sortedReduceModes = reduceModes;
			std::sort(sortedReduceModes.begin(), sortedReduceModes.end());

			for(j = sortedReduceModes.size() - 1; j < sortedReduceModes.size(); j--){
				resDist.erase(resDist.begin() + sortedReduceModes[j]);
			}
			RedistTest newPartialTest;
			newPartialTest.first = resDist;
			newPartialTest.second = partialTest.second;

			bool exists = false;
			for(j = 0; j < fullTests.size(); j++){
				RedistTest check = fullTests[j];
				if(newPartialTest.first == check.first){
					exists = true;
					break;
				}
			}
			if(!exists)
				fullTests.push_back(newPartialTest);
		}

		return;
	}

	std::vector<RedistTest > newPartialTests;
	Mode modeToMove = modesToMove[modesToMove.size() - 1];
	ModeArray newModesToMove = modesToMove;
	newModesToMove.erase(newModesToMove.end() - 1);

	for(i = 0; i < partialTests.size(); i++){
		const TensorDistribution partialDist = partialTests[i].first;
		const ModeArray partialModes = partialTests[i].second;

		for(j = 0; j < sinkModesGroup.size(); j++){
			Mode modeDistToChange = sinkModesGroup[j];
			TensorDistribution resDist = partialDist;
			resDist[modeDistToChange].push_back(modeToMove);
			RedistTest newTest;
			newTest.first = resDist;
			newTest.second = partialModes;

			newPartialTests.push_back(newTest);
		}
	}
//	printf("newPartialTests\n");
//	for(i = 0; i < partialTests.size(); i++){
//		PrintVector(partialTests[i].second, "commModes");
//		std::cout << TensorDistToString(partialTests[i].first) << std::endl;
//	}
//	printf("done\n");
	CreateRSGTestsSinkHelper(newModesToMove, sinkModesGroup, reduceModes, distA, newPartialTests, fullTests);
}

std::vector<RedistTest>
CreateRSGTests(const TensorDistribution& distA){
    Unsigned i, j;
    std::vector<RedistTest > ret;
    const Unsigned order = distA.size() - 1;
    ModeArray tensorModes = DefaultPermutation(order);

    std::vector<ModeArray> redistModesGroups;
    for(i = 1; i < order; i++){
    	std::vector<ModeArray> newRedistModesGroups = AllCombinations(tensorModes, i);
    	redistModesGroups.insert(redistModesGroups.end(), newRedistModesGroups.begin(), newRedistModesGroups.end());
    }

    for(i = 0; i < redistModesGroups.size(); i++){
    	ModeArray redistModesGroup = redistModesGroups[i];
    	TensorDistribution resDist = distA;
    	ModeArray sinkModesGroup;
    	std::set_difference(tensorModes.begin(), tensorModes.end(), redistModesGroup.begin(), redistModesGroup.end(), back_inserter(sinkModesGroup));

    	std::sort(redistModesGroup.begin(), redistModesGroup.end());

    	ModeArray commModes;
    	for(j = redistModesGroup.size() - 1; j < redistModesGroup.size(); j--){
    		Mode redistTenMode = redistModesGroup[j];
    		ModeDistribution redistModeDist = resDist[redistTenMode];
    		commModes.insert(commModes.end(), redistModeDist.begin(), redistModeDist.end());
    		ModeDistribution blank;
    		resDist[redistTenMode] = blank;
    	}

    	std::vector<RedistTest> partialTests;
    	RedistTest partialTest;
    	partialTest.first = resDist;
    	partialTest.second = redistModesGroup;
    	partialTests.push_back(partialTest);

    	CreateRSGTestsSinkHelper(commModes, sinkModesGroup, redistModesGroup, distA, partialTests, ret);
    }
    return ret;
}


#endif // ifndef TMEN_TESTS_RSGREDIST_HPP
