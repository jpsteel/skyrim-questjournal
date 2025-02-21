import gfx.managers.FocusHandler;
import gfx.io.GameDelegate;
import gfx.ui.InputDetails;
import Components.CrossPlatformButtons;
import Shared.GlobalFunc;
import gfx.ui.NavigationCode;
import flash.geom.Rectangle;
import JSON;
import skse;
import QuestsPage;

class QuestJournal extends MovieClip
{
	#include "../version.as"

	var QuestJournal_mc:MovieClip;
	var QuestsList_mc:MovieClip;
	var QuestTitle_mc:MovieClip;
	var QuestTitle:MovieClip;
	var QuestDescription_mc:MovieClip;
	var QuestDescription:MovieClip;
	var TitleKnotwork_mc:MovieClip;
	var TitleKnotwork:MovieClip;
	var BackgroundKnotwork_mc:MovieClip;
	var BackgroundKnotwork:MovieClip;
	var MainQuestHeader_mc:MovieClip;
	var MainQuestHeader:MovieClip;
	var SideQuestHeader_mc:MovieClip;
	var SideQuestHeader:MovieClip;
	var FactionQuestHeader_mc:MovieClip;
	var FactionQuestHeader:MovieClip;
	var MiscQuestHeader_mc:MovieClip;
	var MiscQuestHeader:MovieClip;
	var CompletedQuestHeader_mc:MovieClip;
	var CompletedQuestHeader:MovieClip;
	var FailedQuestHeader_mc:MovieClip;
	var FailedQuestHeader:MovieClip;
	var ObjectivesList_mc:MovieClip;
	var DebugText_mc:MovieClip;
	var QuestsScrollMask_mc:MovieClip;
	var ObjectivesScrollMask_mc:MovieClip;
	var	MiscScrollMask_mc:MovieClip;
	var MiscScrollMask:MovieClip;
	var LocationIconLarge_mc:MovieClip;
	var LocationText_mc:MovieClip;
	var LocationText:MovieClip;
	var ProvinceText_mc:MovieClip;
	var ProvinceText:MovieClip;
	var MenuHeader_mc:MovieClip;
	var MiscQuestsHeader_mc:MovieClip;
	var MiscQuestsScreen_mc:MovieClip;
	var MiscQuestsScreen:MovieClip;
	var MiscObjectivesHolder_mc:MovieClip;
	var MiscObjectivesHolder:MovieClip;
	var MiscQuestsOverlayBackground_mc:MovieClip;
	var debugTextField;
	var hideCompletedTextField;
	var scrollbar:MovieClip;

	var QuestsList = new Array();
	var aMainQuests = new Array();
	var aSideQuests = new Array();
	var aFactionQuests = new Array();
	var aMiscQuests = new Array();
	var aCompletedQuests = new Array();
	var aFailedQuests = new Array();
	var MiscQuest = new Object();

	var aQuestClipsContainer = new Array();
	var aMiscQuestClipsContainer = new Array();
	var aHiddenQuestClipsContainer = new Array();
	var iCurrentQuestIndex:Number;
	var iCurrentMiscQuestIndex:Number;

	var toggleActiveButton:MovieClip;
	var showMapButton:MovieClip;
	var showMapText:MovieClip;
	var showMiscButton:MovieClip;
	var	showMiscText:MovieClip;
	var hideCompleteButton:MovieClip;

	var scrollAmount:Number = 40;
	var questsListHeight:Number;
	var miscObjectivesListHeight:Number;
	var miscObjectivesListBaseY:Number;
	var objectivesListHeight:Number;
	var objectivesListBaseY:Number;

	var bGamepad:Boolean;
	var bUpdated:Boolean;
	var iFocusState:Number;
	var currentQuest:Object;
	var sLastDisplayed:String;
	var aViewedQuests = new Array();
	var bPlayerIsStormcloak:Boolean;
	var bPlayerIsVolkihar:Boolean;
	var bHideCompleted:Boolean;
	var bInitQuestsLoad:Boolean;
	var aMainIDs = new Array();
	var aFactionIDs = new Array();
	var iPlatform;

	function QuestJournal()
	{
		super();
		FocusHandler.instance.setFocus(this,0);
		Mouse.addListener(this);

		debugTextField = DebugText_mc.debugTextField;

		QuestTitle = QuestTitle_mc;
		QuestDescription = QuestDescription_mc;
		TitleKnotwork = TitleKnotwork_mc;
		TitleKnotwork.gotoAndStop("main");
		BackgroundKnotwork = BackgroundKnotwork_mc;
		BackgroundKnotwork.gotoAndStop("main");
		BackgroundKnotwork._alpha = 10;
		LocationIconLarge_mc._xscale = 250;
		LocationIconLarge_mc._yscale = 250;
		LocationText = LocationText_mc;
		ProvinceText = ProvinceText_mc;
		MiscQuestsHeader_mc.gotoAndStop("Favor");
		MiscQuestsScreen = MiscQuestsScreen_mc;
		MiscQuestsScreen.header.gotoAndStop("Favor");
		MiscQuestsScreen._visible = false;
		MiscQuestsOverlayBackground_mc._visible = false;
		MiscQuestsScreen.backgroundKnot.gotoAndStop("Favor");
		MiscQuestsScreen.backgroundKnot._alpha = 10;

		MainQuestHeader = QuestsList_mc.MainQuestHeader_mc;
		SideQuestHeader = QuestsList_mc.SideQuestHeader_mc;
		FactionQuestHeader = QuestsList_mc.FactionQuestHeader_mc;
		CompletedQuestHeader = QuestsList_mc.CompletedQuestHeader_mc;
		FailedQuestHeader = QuestsList_mc.FailedQuestHeader_mc;
		MiscObjectivesHolder = MiscQuestsScreen_mc.MiscObjectivesHolder_mc;

		MainQuestHeader.QuestHeaderKnotwork.gotoAndStop("Main");
		MainQuestHeader.QuestHeaderText.textField.text = "$MAIN";
		SideQuestHeader.QuestHeaderKnotwork.gotoAndStop("Misc");
		SideQuestHeader.QuestHeaderText.textField.text = "$SIDE";
		FactionQuestHeader.QuestHeaderKnotwork.gotoAndStop("Factions");
		FactionQuestHeader.QuestHeaderText.textField.text = "$FACTIONS";
		CompletedQuestHeader.QuestHeaderKnotwork.gotoAndStop("Completed");
		CompletedQuestHeader.QuestHeaderText.textField.text = "$COMPLETED";
		FailedQuestHeader.QuestHeaderKnotwork.gotoAndStop("Failed");
		FailedQuestHeader.QuestHeaderText.textField.text = "$FAILED";

		QuestsList_mc.setMask(QuestsScrollMask_mc);
		QuestsScrollMask_mc._visible = false;

		ObjectivesList_mc.setMask(ObjectivesScrollMask_mc);
		ObjectivesScrollMask_mc._visible = false;
		objectivesListBaseY = ObjectivesList_mc._y;
		
		MiscScrollMask = MiscQuestsScreen_mc.MiscScrollMask_mc;
		MiscObjectivesHolder.setMask(MiscScrollMask);
		MiscScrollMask._visible = false;
		miscObjectivesListBaseY = MiscObjectivesHolder._y;
		
		iFocusState = 0;
		bUpdated = false;
		bInitQuestsLoad = true;
		iCurrentQuestIndex = -1;
	}

	function onLoad()
	{
	}

	function handleInput(details:InputDetails, pathToFocus:Array):Boolean
	{
		var bHandledInput:Boolean = false;
		if (GlobalFunc.IsKeyPressed(details))
		{
			if (details.navEquivalent == NavigationCode.RIGHT)
			{
				details.navEquivalent = NavigationCode.ENTER;
			}
			//debugLog("keycode: "+details.code); 
			//debugLog("navEquivalent: "+details.navEquivalent);
			if (details.code == 77 || details.navEquivalent == NavigationCode.GAMEPAD_Y)
			{
				if (iFocusState == 0){
					if (currentQuest.objectives[0].questTargetID == undefined){
						GameDelegate.call("PlaySound",["UIMenuCancel"]);
					} else {
						GameDelegate.call("ShowQuestOnMap",[Number(currentQuest.objectives[0].questTargetID)]);
					}
				} else if (iFocusState == 1){
					if (aMiscQuestClipsContainer[iCurrentMiscQuestIndex].objective.questTargetID == undefined) {
						GameDelegate.call("PlaySound",["UIMenuCancel"]);
					} else {
						GameDelegate.call("ShowQuestOnMap",[Number(aMiscQuestClipsContainer[iCurrentMiscQuestIndex].objective.questTargetID)]);
					}
				}
				bHandledInput = true;
			}
			else if ((details.code == 72 || details.navEquivalent == NavigationCode.GAMEPAD_R3) && MiscQuestsScreen._visible == false)
			{
				bHideCompleted = !bHideCompleted;
				SetHideCompletedText(bHideCompleted);
				for (var i in QuestsList_mc)
				{
					QuestsList_mc[i].removeMovieClip();
				}
				SetQuests(QuestsList);
				if (iCurrentQuestIndex > aQuestClipsContainer.length - 1){
					iCurrentQuestIndex = aQuestClipsContainer.length - 1
					onQuestHover(aQuestClipsContainer[iCurrentQuestIndex],aQuestClipsContainer[iCurrentQuestIndex].quest);
					if (aQuestClipsContainer[iCurrentQuestIndex]._y + QuestsList_mc._y > 260)
					{
						QuestsList_mc._y = 260 - aQuestClipsContainer[iCurrentQuestIndex]._y;
					} else if (aQuestClipsContainer[iCurrentQuestIndex]._y + QuestsList_mc._y < - 210)
					{
						QuestsList_mc._y = - 210 - aQuestClipsContainer[iCurrentQuestIndex]._y;
					}
				}
				bHandledInput = true;
			}
			else if ((details.code == 81 || details.navEquivalent == NavigationCode.GAMEPAD_X) && MiscQuestsScreen._visible == false)
			{
				MiscQuestsScreen._visible = true;
				MiscQuestsOverlayBackground_mc._visible = true;
				GameDelegate.call("PlaySound",["UIMenuBladeOpenSD"]);
				showMiscText.text = "$MISC_BUTTON_HIDE";
				iFocusState = 1;
				if (iCurrentMiscQuestIndex < aMiscQuestClipsContainer.length){
					onMiscObjectiveHover(aMiscQuestClipsContainer[iCurrentMiscQuestIndex], aMiscQuestClipsContainer[iCurrentMiscQuestIndex].objective)
				} else {
					showMapButton._alpha = 50;
					showMapText._alpha = 50;
				}
				bHandledInput = true;
			}
			else if ((details.code == 81 || details.navEquivalent == NavigationCode.GAMEPAD_X) && MiscQuestsScreen._visible == true)
			{
				MiscQuestsScreen._visible = false;
				MiscQuestsOverlayBackground_mc._visible = false;
				GameDelegate.call("PlaySound",["UIMenuBladeCloseSD"]);
				showMiscText.text = "$MISC_BUTTON_SHOW";
				iFocusState = 0;
				if (currentQuest == {}){
					showMapButton._alpha = 50;
					showMapText._alpha = 50;
				} else if (iCurrentQuestIndex == -1) {
					if (aQuestClipsContainer[0].quest.objectives[0].questTargetID == undefined){
						showMapButton._alpha = 50;
						showMapText._alpha = 50;
					} else {
						showMapButton._alpha = 100;
						showMapText._alpha = 100;
					}
				} else {
					onQuestHover(aQuestClipsContainer[iCurrentQuestIndex],aQuestClipsContainer[iCurrentQuestIndex].quest);
				}
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.UP)
			{
				if (iFocusState == 0){
					if (iCurrentQuestIndex > 0)
					{
						onQuestHover(aQuestClipsContainer[iCurrentQuestIndex - 1],aQuestClipsContainer[iCurrentQuestIndex - 1].quest);
						if (iCurrentQuestIndex == 0)
						{
							QuestsList_mc._y = - 50;
						} 
						else if (aQuestClipsContainer[iCurrentQuestIndex]._y + QuestsList_mc._y < - 236)
						{
							QuestsList_mc._y = - 236 - aQuestClipsContainer[iCurrentQuestIndex]._y;
						}
					}
				} else if (iFocusState == 1){
					if (iCurrentMiscQuestIndex < aMiscQuestClipsContainer.length - 1){
						onMiscObjectiveHover(aMiscQuestClipsContainer[iCurrentMiscQuestIndex + 1], aMiscQuestClipsContainer[iCurrentMiscQuestIndex + 1].objective);
						if (iCurrentMiscQuestIndex == aMiscQuestClipsContainer.length - 1)
						{
							MiscObjectivesHolder._y = - 125;
						} 
						else if (aMiscQuestClipsContainer[iCurrentMiscQuestIndex]._y + MiscObjectivesHolder._y < - 125)
						{
							MiscObjectivesHolder._y += 40;
						}
					}
				}
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.DOWN)
			{
				if (iFocusState == 0){
					if (iCurrentQuestIndex < aQuestClipsContainer.length - 1)
					{
						onQuestHover(aQuestClipsContainer[iCurrentQuestIndex + 1],aQuestClipsContainer[iCurrentQuestIndex + 1].quest);
						if (aQuestClipsContainer[iCurrentQuestIndex]._y + QuestsList_mc._y > 264)
						{
							QuestsList_mc._y = 264 - aQuestClipsContainer[iCurrentQuestIndex]._y;
						}
					}
				} else if (iFocusState == 1){
					if (iCurrentMiscQuestIndex > 0){
						onMiscObjectiveHover(aMiscQuestClipsContainer[iCurrentMiscQuestIndex - 1], aMiscQuestClipsContainer[iCurrentMiscQuestIndex - 1].objective);
						if (aMiscQuestClipsContainer[iCurrentMiscQuestIndex]._y + MiscObjectivesHolder._y > 155)
						{
							MiscObjectivesHolder._y = 155 - aMiscQuestClipsContainer[iCurrentMiscQuestIndex]._y;
						}
					}
				}
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.ENTER)
			{
				if (iFocusState == 0){
					if (aQuestClipsContainer[iCurrentQuestIndex].quest.category != "completed" && iCurrentQuestIndex >= 0)
					{
						onQuestSelected(aQuestClipsContainer[iCurrentQuestIndex],aQuestClipsContainer[iCurrentQuestIndex].quest);
						if (aQuestClipsContainer[iCurrentQuestIndex]._y + QuestsList_mc._y > 264)
						{
							QuestsList_mc._y = 264 - aQuestClipsContainer[iCurrentQuestIndex]._y;
						} else if (aQuestClipsContainer[iCurrentQuestIndex]._y + QuestsList_mc._y < - 236)
						{
							QuestsList_mc._y = - 236 - aQuestClipsContainer[iCurrentQuestIndex]._y;
						}
					}
				} else if (iFocusState == 1){
					if(iCurrentMiscQuestIndex < aMiscQuestClipsContainer.length){
						OnMiscObjectiveSelected(aMiscQuestClipsContainer[iCurrentMiscQuestIndex], aMiscQuestClipsContainer[iCurrentMiscQuestIndex].objective);
						if (aMiscQuestClipsContainer[iCurrentMiscQuestIndex]._y + MiscObjectivesHolder._y > 155)
						{
							MiscObjectivesHolder._y = 155 - aMiscQuestClipsContainer[iCurrentMiscQuestIndex]._y;
						}
						else if (aMiscQuestClipsContainer[iCurrentMiscQuestIndex]._y + MiscObjectivesHolder._y < - 125)
						{
							MiscObjectivesHolder._y = -125 - aMiscQuestClipsContainer[iCurrentMiscQuestIndex]._y;
						}
					}
				}
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.GAMEPAD_R2)
			{
				if (iFocusState == 0){
					if (ObjectivesList_mc._y + ObjectivesList_mc._height > ObjectivesScrollMask_mc._y + ObjectivesScrollMask_mc._height)
					{
						ObjectivesList_mc._y -= scrollAmount;
					}
				}
				bHandledInput = true;
			}
			else if (details.navEquivalent == NavigationCode.GAMEPAD_L2)
			{
				if (iFocusState == 0){
					if (ObjectivesList_mc._y < ObjectivesScrollMask_mc._y)
					{
						ObjectivesList_mc._y += scrollAmount;
					}
				}
				bHandledInput = true;
			}
		}
		return bHandledInput;
	}

	function onMouseWheel(delta:Number):Void
	{
		var pt:Object = {_x:_xmouse, _y:_ymouse};
		var tolerance:Number = 0.1;// Tolerance for floating-point comparison
		var minScrollY:Number;

		if (iFocusState == 0)
		{
			if (pt._x >= QuestsScrollMask_mc._x && pt._x <= QuestsScrollMask_mc._x + QuestsScrollMask_mc._width && pt._y >= QuestsScrollMask_mc._y && pt._y <= QuestsScrollMask_mc._y + QuestsScrollMask_mc._height)
			{
				minScrollY = QuestsScrollMask_mc._y + QuestsScrollMask_mc._height - questsListHeight;
				if ((Math.abs(QuestsList_mc._y - (QuestsScrollMask_mc._y + 155)) < tolerance && delta > 0) || (questsListHeight - QuestsScrollMask_mc._height < tolerance && delta < 0))
				{
					return;
				}
				else
				{
					QuestsList_mc._y += delta * scrollAmount;

					if (QuestsList_mc._y > QuestsScrollMask_mc._y + 155)
					{
						QuestsList_mc._y = QuestsScrollMask_mc._y + 155;
					}

					if (QuestsList_mc._y < minScrollY)
					{
						QuestsList_mc._y = minScrollY;
					}
				}
				return;
			}

			if (pt._x >= ObjectivesScrollMask_mc._x && pt._x <= ObjectivesScrollMask_mc._x + ObjectivesScrollMask_mc._width && pt._y >= ObjectivesScrollMask_mc._y && pt._y <= ObjectivesScrollMask_mc._y + ObjectivesScrollMask_mc._height)
			{
				minScrollY = ObjectivesScrollMask_mc._y + 5 + ObjectivesScrollMask_mc._height - objectivesListHeight;
				if ((Math.abs(ObjectivesList_mc._y - ObjectivesScrollMask_mc._y - 5) < tolerance && delta > 0) || (objectivesListHeight - ObjectivesScrollMask_mc._height < tolerance && delta < 0))
				{
					return;
				}
				else
				{
					ObjectivesList_mc._y += delta * scrollAmount;

					if (ObjectivesList_mc._y > ObjectivesScrollMask_mc._y + 5)
					{
						ObjectivesList_mc._y = ObjectivesScrollMask_mc._y + 5;
					}
					
					if (ObjectivesList_mc._y < minScrollY)
					{
						ObjectivesList_mc._y = minScrollY;
					}
				}
				return;
			}
		}
		else if (iFocusState ==1)
		{
			if (pt._x >= MiscScrollMask._x && pt._x <= MiscScrollMask._x + MiscScrollMask._width && pt._y >= MiscScrollMask._y && pt._y <= MiscScrollMask._y + MiscScrollMask._height)
			{
				minScrollY = MiscScrollMask._y + MiscScrollMask._height - miscObjectivesListHeight;
				if ((Math.abs(MiscObjectivesHolder._y - MiscScrollMask._y) < tolerance && delta > 0) || (miscObjectivesListHeight - MiscScrollMask._height < tolerance && delta < 0))
				{
					return;
				}
				else
				{
					MiscObjectivesHolder._y += delta * scrollAmount;

					if (MiscObjectivesHolder._y > MiscScrollMask._y)
					{
						MiscObjectivesHolder._y = MiscScrollMask._y;
					}

					if (MiscObjectivesHolder._y < minScrollY)
					{
						MiscObjectivesHolder._y = minScrollY;
					}
				}
				return;
			}
		}
		return;
	}

	function DisplayFirstQuest()
	{
		if (currentQuest == undefined){
			if (aQuestClipsContainer.length > 0)
			{
				currentQuest = aQuestClipsContainer[0].quest;
				iCurrentQuestIndex = -1;
			}
			else
			{
				currentQuest = {};
				QuestTitle.textField.text = "";
				QuestDescription.textField.text = "No Active Quests";
				QuestDescription.textField.textColor = 0x999999;
				LocationIconLarge_mc.gotoAndStop("none");
				SetLocationText("");
				TitleKnotwork.gotoAndStop("Main");
				BackgroundKnotwork.gotoAndStop("Main");
				return;
			}
		}
		DisplayQuestData(currentQuest);
		if (aQuestClipsContainer[iCurrentQuestIndex]._y + QuestsList_mc._y > 264)
		{
			QuestsList_mc._y = 264 - aQuestClipsContainer[iCurrentQuestIndex]._y;
		} else if (aQuestClipsContainer[iCurrentQuestIndex]._y + QuestsList_mc._y < - 236)
		{
			QuestsList_mc._y = - 236 - aQuestClipsContainer[iCurrentQuestIndex]._y;
		}
	}

	function onQuestHover(questClip:MovieClip, quest:Object)
	{
		currentQuest = quest;
		onQuestRollOut(aQuestClipsContainer[iCurrentQuestIndex],aQuestClipsContainer[iCurrentQuestIndex].quest);
		iCurrentQuestIndex = questClip.index;
		if (iFocusState != 0)
		{
			return;
		}
		GameDelegate.call("PlaySound",["UIMenuFocus"]);
		
		if (currentQuest.active == "true")
		{
			questClip.gotoAndStop("activeHover");
		}
		else
		{
			questClip.gotoAndStop("inactiveHover");
		}
		
		if(questClip.viewed == 0){
			questClip.viewed = 1;
			questClip.newQuestIndicator._visible = false;
			aViewedQuests.push(quest.title);
		} else {
			questClip.newQuestIndicator._visible = false;
		}
		DisplayQuestData(currentQuest);
	}

	function DisplayQuestData(quest)
	{
		QuestTitle.textField.text = quest.title.toUpperCase();
		QuestDescription.textField.text = quest.description;
		QuestDescription.textField.textAutoSize = "shrink";
		GameDelegate.call("RequestQuestLocation",[]);
		if (quest.location.substr(-8, 8).toUpperCase() == "OBLIVION")
		{
			LocationIconLarge_mc.gotoAndStop("realmOblivion");
		}
		LocationIconLarge_mc.gotoAndStop(quest.location);
		SetLocationText(quest.location);
		TitleKnotwork.gotoAndStop(quest.type);
		BackgroundKnotwork.gotoAndStop(quest.type);
		SetObjectives(quest);
		if (quest.objectives[0].questTargetID == undefined){
			showMapButton._alpha = 50;
			showMapText._alpha = 50;
		} else {
			showMapButton._alpha = 100;
			showMapText._alpha = 100;
		}
	}

	function onQuestRollOut(questClip:MovieClip, quest:Object)
	{
		if (iFocusState != 0)
		{
			return;
		}
		if (quest.active == "true")
		{
			questClip.gotoAndStop("active");
		}
		else
		{
			questClip.gotoAndStop("inactive");
		}
	}

	function onQuestSelected(questClip:MovieClip, quest:Object)
	{
		if (iFocusState != 0)
		{
			return;
		}

		if (quest.active == "true")
		{
			questClip.gotoAndStop("inactiveHover");
			//questClip.location.gotoAndStop(quest.location);
			GameDelegate.call("PlaySound",["UIQuestInactive"]);
			quest.active = "false";
			questClip.newQuestIndicator._visible = false;
		}
		else
		{
			questClip.gotoAndStop("activeHover");
			//questClip.location.gotoAndStop(quest.location + "Active");
			GameDelegate.call("PlaySound",["UIQuestActive"]);
			quest.active = "true";
		}
		GameDelegate.call("ToggleQuestActive",[quest.formID]);
		SetObjectives(quest);
		questClip.location.gotoAndStop("none");
		questClip.textField.text = quest.title;
	}

	function SetQuests(quests:Array):Void
	{
		var i:Number;
		aQuestClipsContainer = [];
		//Quest types:
		//0-Misc 1-Main 2-MagesGuild 3-ThievesGuild 4-DarkBrotherhood 5-Companions 6-Favor 7-Daedric 8-Secondary 9-CivilWarEmpire 10-DLC01Vampire 11-DLC02 12-CivilWarSons 13-DLC01Dawnguard 14-Complete 15-Failed
		//Knotwork mod types:
		//16-Fishing 17-EastEmpireCompany 18-BardsCollegeA 19-BardsCollegeB 20-PenitusOculatus 21-LotDMuseum 22-LotDExplorers 23-Falskaar 24-VigilantsStendarr 25-Cogs 26-Cyrodiil
		if (bInitQuestsLoad){
			for (i = 0; i < QuestsList.length; i++)
			{
				QuestsList[i].text = QuestsList[i].text.toUpperCase();
				var qst = {title:QuestsList[i].text, type:Number(QuestsList[i].type), description:QuestsList[i].description, formID:QuestsList[i].formID, active:QuestsList[i].active, objectives:QuestsList[i].objectives, instance:QuestsList[i].instance, timeIndex:QuestsList[i].timeIndex, location:QuestsList[i].location};
	
				if (qst.title == undefined)
				{
					continue;
				}
				if (bPlayerIsStormcloak && qst.type == 9){
					qst.type = 12;
				}
				if (!bPlayerIsVolkihar && qst.type == 10){
					qst.type = 13;
				}
				if (qst.type == 0 || qst.type == 6){
					qst.type = 8;
				}
				
				if (QuestsList[i].completed == 1)
				{
					qst.category = "completed";
					if (qst.objectives[qst.objectives.length - 1].failed == 1 || qst.objectives[qst.objectives.length - 1].completed == 0)
					{
						qst.objectives[qst.objectives.length - 1].failed = 1;
						aFailedQuests.push(qst);
					}
					else
					{
						aCompletedQuests.push(qst);
					}
				}
				else if (qst.title == "$MISCELLANEOUS")
				{
					qst.category = "misc";
					SetMiscObjectives(qst);
				}
				else
				{
					if (containsElement(aMainIDs, qst.type)){
						qst.category = "main";
						aMainQuests.push(qst);
						aMainQuests.sortOn("timeIndex");
					} else if (containsElement(aFactionIDs, qst.type)){
						qst.category = "faction";
						aFactionQuests.push(qst);
						aFactionQuests.sortOn("timeIndex");
					} else if (qst.type == undefined){
						qst.type = "8";
						qst.category = "side";
						aSideQuests.push(qst);
						aSideQuests.sortOn("timeIndex");
					} else {
						qst.category = "side";
						aSideQuests.push(qst);
						aSideQuests.sortOn("timeIndex");
					}
				}
			}
		}
		
		PopulateQuestCategory(MainQuestHeader,aMainQuests,undefined,undefined,false);
		PopulateQuestCategory(SideQuestHeader,aSideQuests,MainQuestHeader,aMainQuests,false);
		PopulateQuestCategory(FactionQuestHeader,aFactionQuests,SideQuestHeader,aSideQuests,false);
		if (bHideCompleted){
			CompletedQuestHeader._visible = false;
			FailedQuestHeader._visible = false;
			questsListHeight = FactionQuestHeader._y + FactionQuestHeader._height + (aFactionQuests.length * 40) - MainQuestHeader._y  - 125;
		} else if (aFailedQuests.length > 0){
			CompletedQuestHeader._visible = true;
			FailedQuestHeader._visible = true;
			PopulateQuestCategory(CompletedQuestHeader,aCompletedQuests,FactionQuestHeader,aFactionQuests,true);
			PopulateQuestCategory(FailedQuestHeader,aFailedQuests,CompletedQuestHeader,aCompletedQuests,true);
			questsListHeight = FailedQuestHeader._y + FailedQuestHeader._height + (aFailedQuests.length * 40) - MainQuestHeader._y  - 125;
		} else {
			CompletedQuestHeader._visible = true;
			PopulateQuestCategory(CompletedQuestHeader,aCompletedQuests,FactionQuestHeader,aFactionQuests,true);
			FailedQuestHeader._visible = false;
			questsListHeight = CompletedQuestHeader._y + CompletedQuestHeader._height + (aCompletedQuests.length * 40) - MainQuestHeader._y  - 125;
		}
		bInitQuestsLoad = false;
	}

	function PopulateQuestCategory(questHeader:MovieClip, questsList:Array, previousCategoryHeader:MovieClip, previousQuestsList:Array, complete:Boolean):Void
	{
		if (previousCategoryHeader != undefined)
		{
			questHeader._y = previousCategoryHeader._y + 50 + (40 * previousQuestsList.length);
		}
		for (var i = 0; i < questsList.length; i++)
		{
			var quest = questsList[i];
			var questName = quest.title.toUpperCase();

			var offset = 40 * i;
			//QuestsList_mc.createEmptyMovieClip("containerFailed" + i.toString(), QuestsList_mc.getNextHighestDepth());
			var questClip:MovieClip = QuestsList_mc.attachMovie("questSelector", "quest" + i.toString(), QuestsList_mc.getNextHighestDepth(), {_x:questHeader._x - 50, _y:questHeader._y + 45 + offset});
			questClip.quest = quest;
			if (!complete)
			{
				if (quest.active == "true")
				{
					questClip.gotoAndStop("active");
					//questClip.location.gotoAndStop(quest.location + "Active");
				}
				else
				{
					questClip.gotoAndStop("inactive");
					//questClip.location.gotoAndStop(quest.location);
				}
				questClip.onPress = function()
				{
					_parent._parent.onQuestSelected(this,this.quest);
				};
			}
			else
			{
				questClip.gotoAndStop("inactive");
				//questClip.location.gotoAndStop(quest.location + "Complete");
				questClip.textField.textColor = 0x999999;
			}
			questClip.location.gotoAndStop("none");
			questClip.textField.text = questName;


			questClip.onRollOver = function()
			{
				_parent._parent.onQuestHover(this,this.quest);
			};
			questClip.onRollOut = function()
			{
				_parent._parent.onQuestRollOut(this,this.quest);
			};
			questClip.index = aQuestClipsContainer.length;
			aQuestClipsContainer.push(questClip);
			if (quest.formID == sLastDisplayed && bInitQuestsLoad){
				currentQuest = quest;
				iCurrentQuestIndex = questClip.index;
			}
			if (!containsElement(aViewedQuests, quest.title) && quest.category != "completed") {
				questClip.viewed = 0;
				questClip.newQuestIndicator._visible = true;
			} else {
				questClip.viewed = 1;
				questClip.newQuestIndicator._visible = false;
			}
		}
	}

	function SetObjectives(quest:Object)
	{
		var offset:Number;
		objectivesListHeight = 0;
		var Objectives:Array = quest.objectives;
		for (var j in ObjectivesList_mc)
		{
			ObjectivesList_mc[j].removeMovieClip();
		}
		for (var i in Objectives)
		{
			offset = 40 * i;
			//ObjectivesList.createEmptyMovieClip("containerObjectives" + i.toString(), ObjectivesList.getNextHighestDepth());
			var objectiveClip:MovieClip = ObjectivesList_mc.attachMovie("objectiveHolder", "objective" + i.toString(), ObjectivesList_mc.getNextHighestDepth(), {_x:this._parent._x + 40, _y:this._parent._y + offset});
			objectiveClip.ObjectiveText.textField.text = Objectives[i].text;
			objectiveClip.ObjectiveText.textField.textColor = 0x999999;
			if (Objectives[i].completed == "1")
			{
				objectiveClip.ObjectiveStatus.gotoAndStop("completed");
				objectivesListHeight += 1;
			}
			else if (Objectives[i].completed == "0" && Objectives[i].failed == "0")
			{
				if (quest.active == "true")
				{
					objectiveClip.ObjectiveStatus.gotoAndStop("highlighted");
					objectiveClip.ObjectiveText.textField.textColor = 0xFFFFFF;
				}
				else
				{
					objectiveClip.ObjectiveStatus.gotoAndStop("displayed");
				}
				objectivesListHeight += 1;
			}
			else if (Objectives[i].failed == "1")
			{
				objectiveClip.ObjectiveStatus.gotoAndStop("failed");
				objectivesListHeight += 1;
			}
		}
		objectivesListHeight = objectivesListHeight * 40;
		ObjectivesList_mc._y = objectivesListBaseY;
	}

	function SetMiscObjectives(quest:Object)
	{
		var offset:Number;
		miscObjectivesListHeight = 0;
		var Objectives:Array = quest.objectives;
		for (var j in MiscObjectivesHolder)
		{
			MiscObjectivesHolder[j].removeMovieClip();
		}
		for (var i in Objectives)
		{
			offset = 40 * i;
			//ObjectivesList.createEmptyMovieClip("containerObjectives" + i.toString(), ObjectivesList.getNextHighestDepth());
			var objectiveClip:MovieClip = MiscObjectivesHolder.attachMovie("objectiveHolder", "objective" + i.toString(), MiscObjectivesHolder.getNextHighestDepth(), {_x:this._parent._x + 40, _y:this._parent._y + offset});
			objectiveClip.ObjectiveText.textField.text = Objectives[i].text;
			if (Objectives[i].active == "true")
			{
				objectiveClip.ObjectiveStatus.gotoAndStop("miscActive");
			}
			else if (Objectives[i].active == "false")
			{
				objectiveClip.ObjectiveStatus.gotoAndStop("displayed");
			}
			objectiveClip.ObjectiveText.textField.textColor = 0x999999;
			miscObjectivesListHeight += 1;

			objectiveClip.objective = Objectives[i];
			objectiveClip.onRollOver = function()
			{
				_parent._parent._parent.onMiscObjectiveHover(this,this.objective);
			};
			objectiveClip.onRollOut = function()
			{
				_parent._parent._parent.onMiscObjectiveRollOut(this,this.objective);
			};
			objectiveClip.onPress = function()
			{
				_parent._parent._parent.OnMiscObjectiveSelected(this,this.objective);
			};
			objectiveClip.index = aMiscQuestClipsContainer.length;
			aMiscQuestClipsContainer.push(objectiveClip);
		}
		miscObjectivesListHeight = miscObjectivesListHeight * 40;
		MiscObjectivesHolder._y = miscObjectivesListBaseY;
		iCurrentMiscQuestIndex = aMiscQuestClipsContainer.length;
	}
	
	function onMiscObjectiveHover(objectiveClip:MovieClip, objective:Object){
		onMiscObjectiveRollOut(aMiscQuestClipsContainer[iCurrentMiscQuestIndex],aMiscQuestClipsContainer[iCurrentMiscQuestIndex].objective);
		iCurrentMiscQuestIndex = objectiveClip.index;
		GameDelegate.call("PlaySound",["UIMenuFocus"]);
		if (objective.active == "false"){
			objectiveClip.ObjectiveStatus.gotoAndStop("highlighted");
		} else {
			objectiveClip.ObjectiveStatus.gotoAndStop("miscActiveHighlighted");
		}
		
		if (objective.questTargetID == undefined){
			showMapButton._alpha = 50;
			showMapText._alpha = 50;
		} else {
			showMapButton._alpha = 100;
			showMapText._alpha = 100;
		}
		objectiveClip.ObjectiveText.textField.textColor = 0xFFFFFF;
	}
	
	function onMiscObjectiveRollOut(objectiveClip:MovieClip, objective:Object){
		objectiveClip.ObjectiveText.textField.textColor = 0x999999;
		if (objective.active == "false"){
			objectiveClip.ObjectiveStatus.gotoAndStop("displayed");
		} else {
			objectiveClip.ObjectiveStatus.gotoAndStop("miscActive");
		}
	}

	function OnMiscObjectiveSelected(objectiveClip:MovieClip, objective:Object)
	{
		objectiveClip.ObjectiveText.textField.textColor = 0xFFFFFF;
		if (objective.active == "true")
		{
			objectiveClip.ObjectiveStatus.gotoAndStop("highlighted");
			GameDelegate.call("PlaySound",["UIQuestInactive"]);
			objective.active = "false";
		}
		else
		{
			objectiveClip.ObjectiveStatus.gotoAndStop("miscActiveHighlighted");
			GameDelegate.call("PlaySound",["UIQuestActive"]);
			objective.active = "true";
		}
		GameDelegate.call("ToggleQuestActive",[objective.formID]);
	}

	function onQuestsDataComplete(questsData, hideCompleted, mainIDs, factionIDs)
	{
		if (!bUpdated)
		{
			bUpdated = true;
		
			if (hideCompleted != undefined){
				bHideCompleted = hideCompleted;
			} else {
				bHideCompleted = false;
			}
			SetHideCompletedText(bHideCompleted);
			aMainIDs = mainIDs.split(";");
			aFactionIDs = factionIDs.split(";");
			
			for (var i:Number = 0; i < questsData.length; i++)
			{
				var questObject:Object = {};
				for (var attr in questsData[i])
				{
					var dat = questsData[i][attr];
					if (attr.toString() == "objectives")
					{
						var objectivesArray:Array = [];
						for (var j:Number = 0; j < questsData[i][attr].length; j++)
						{
							var objectiveObject:Object = {};
							var objective = questsData[i][attr][j];
							for (var elt in objective)
							{
								objectiveObject[elt.toString()] = objective[elt].toString();
							}
							objectivesArray.push(objectiveObject);
						}
						questObject["objectives"] = objectivesArray;
					}
					else
					{
						questObject[attr.toString()] = dat.toString();
					}
				}
				QuestsList.push(questObject);
			}
			var serializedQuestsList:String = JSON.stringify(QuestsList);
			SetQuests(QuestsList);
			DisplayFirstQuest();
		}
		else
		{
			return;
		}
	}

	function SetHeaderInfo(playerLevel, xpProgress, datetime)
	{
		MenuHeader_mc.LevelMeterRect.LevelNumberLabel.text = playerLevel;
		MenuHeader_mc.LevelMeterRect.LevelNumberLabel.textAutoSize = "shrink";
		MenuHeader_mc.LevelMeterRect.LevelProgressBar.gotoAndStop(140 - xpProgress);
		MenuHeader_mc.DateText.text = datetime;
	}

	function SetGamepad(gamepad)
	{
		bGamepad = gamepad;
		if (!bGamepad){
			toggleActiveButton.gotoAndStop(28);
			showMapButton.gotoAndStop(50);
			showMiscButton.gotoAndStop(16);
			hideCompleteButton.gotoAndStop(35);
		}else{
			toggleActiveButton.gotoAndStop(276);
			showMapButton.gotoAndStop(279);
			showMiscButton.gotoAndStop(278);
			hideCompleteButton.gotoAndStop(273);
		}
	}

	function SetLocationText(location)
	{
		var questLoc = location.toUpperCase();
		var regionString:String;
		var provinceString:String;
		if (questLoc.substr(-4, 4) == "HOLD")
		{
			var hold = questLoc.substr(0, length(questLoc) - 4);
			regionString = "$" + hold + "_HOLD";
			provinceString = "$SKYRIM";
		}
		else if (questLoc.substr(-11, 11) == "TERRITORIES")
		{
			var territory = questLoc.substr(0, length(questLoc) - 11);
			regionString = "$" + territory + "_TERRITORIES";
			provinceString = "$MORROWIND";
		}
		else if (questLoc.substr(-6, 6) == "COUNTY")
		{
			var county = questLoc.substr(0, length(questLoc) - 6);
			regionString = "$" + county + "_COUNTY";
			provinceString = "$CYRODIIL";
		}
		else if (questLoc.substr(-8, 8) == "OBLIVION")
		{
			var plane = questLoc.substr(0, length(questLoc) - 8);
			regionString = "$" + plane + "_PLANE";
			provinceString = "$OBLIVION";
		}
		else
		{
			regionString = "";
			provinceString = "";
		}
		
		LocationText.textField.text = regionString;
		LocationText.textField.textAutoSize = "shrink";
		ProvinceText.textField.text = provinceString;
		ProvinceText.textField.textAutoSize = "shrink";
	}
	
	function SetLastDisplayedQuest(questFormID){
		sLastDisplayed = questFormID;
	}
	
	function SetViewedQuests(viewedQuests){
		aViewedQuests = viewedQuests.split(";");
	}
	
	function SetPlayerFactions(stormcloaks, vampires){
		bPlayerIsStormcloak = stormcloaks;
		bPlayerIsVolkihar = vampires;
	}
	
	function SetHideCompletedText(hideCompleted){
		if (!hideCompleted){
			hideCompletedTextField.text = "$HIDE_COMPLETE";
			GameDelegate.call("PlaySound",["UIMenuBladeOpenSD"]);
		} else {
			hideCompletedTextField.text = "$SHOW_COMPLETE";
			GameDelegate.call("PlaySound",["UIMenuBladeCloseSD"]);
		}
	}
	
	function containsElement(arr:Array, element):Boolean {
		for (var i:Number = 0; i < arr.length; i++) {
			if (arr[i] == element) {
				return true;
			}
		}
		return false;
	}
	
	function SetPlatform(a_platform: Number, a_bPS3Switch: Boolean): Void
	{
		iPlatform = a_platform;
	}

	function debugLog(message:Object):Void
	{
		if (typeof (message) == "string")
		{
			debugTextField.text += message + "\n";
		}
		else if (typeof (message) == "object")
		{
			var logText:String = "Object Details: ";
			for (var prop in message)
			{
				logText += prop + ": " + message[prop] + "; ";
			}
			debugTextField.text += logText + "\n";
		}
	}

}