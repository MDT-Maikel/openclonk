
func InitializePlayer(int plr)
{
	CreateObjectAbove(Flagpole, 100,200, plr);
	CreateObjectAbove(SteamEngine, 50,200, plr);
	CreateObjectAbove(Pump, 130,200, plr);
	GetCrew(plr)->CreateContents(Hammer);
	GetCrew(plr)->CreateContents(Wood, 28);
	GetCrew(plr)->CreateContents(Metal, 8);
	GetCrew(plr)->SetPosition(130,190);
	SetPlrKnowledge(plr, WoodenBridge);
	SetPlrKnowledge(plr, WindGenerator);
	return true;
}
