#include"Activity.h"
Activity::Activity(string aid, string cid, string name, 
	string tag, string st, string et, int maxp):activityId(aid), clubId(cid), activityName(name), tags(tag),
	startTime(st), endTime(et), maxPeople(maxp), currentPeople(0){ }
string Activity::getActivityId() const {
	return activityId;
}
string Activity::getClubId() const {
  return clubId;
}
string Activity::getName() const {
	return activityName;
}
string Activity::getTags() const {
	return tags;
}
string Activity::getStartTime() const { 
	return startTime;
}
string Activity::getEndTime() const { 
	return endTime;
}
int Activity::getMax() const { 
	return maxPeople; 
}
int Activity::getCur() const { 
	return currentPeople; 
}
vector<string> Activity::getWaitList() const { 
	return waitList; 
}

void Activity::addCur() { 
	currentPeople++; 
}
void Activity::subCur() { 
	if (currentPeople > 0) currentPeople--;
}
void Activity::addWait(const string& uid) { 
	waitList.push_back(uid); 
}
