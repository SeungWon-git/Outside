#include "ZombieBT.h"

// BT
#include "Task.h"
#include "Selector.h"
#include "Sequence.h"

#include "Detect.h"
#include "CanSeePlayer.h"
#include "HasShouting.h"
#include "HasFootSound.h"
#include "HordeAction.h"
#include "HasInvestigated.h"
#include "NotHasLastKnownPlayerLocation.h"
#include "CanAttack.h"
#include "CanNotAttack.h"
#include "Attack.h"
#include "MoveTo.h"
#include "LookAround.h"


//======[[좀비 BT 생성]]======//
ZombieBT::ZombieBT()
{
	//======[Task] 메모리 할당======//

	//<Detect-Selector>
	sel_detect = new Sel_Detect;
	//<CanSeePlayer-Selector>
	sel_canseeplayer = new Sel_CanSeePlayer;

	//{HasShouting-Sequence}
	seq_hasshouting = new Seq_HasShouting;
	//{HasFootSound-Sequence}
	seq_hasfootsound = new Seq_HasFootSound;
	//{HordeAction-Sequence}
	seq_hordeaction = new Seq_HordeAction;
	//{HasInvestigated-Sequence}
	seq_hasinvestigated = new Seq_HasInvestigated;
	//{NotHasLastKnownPlayerLocation-Sequence}
	seq_nothaslastknownplayerlocation = new Seq_NotHasLastKnownPlayerLocation;

	//{CanNotAttack-Sequence}
	seq_cannotattack = new Seq_CanNotAttack;
	//{CanAttack-Sequence}
	seq_canattack = new Seq_CanAttack;

	//[MoveTo-Task]
	t_moveto = new T_MoveTo;
	//[Attack-Task]
	t_attack = new T_Attack;
	//[LookAround-Task]
	t_lookaround = new T_LookAround;


	MakeZombieBT();

	//==========================//
}

//======== 트리 작성 (작업 할당) ========//
void ZombieBT::MakeZombieBT()
{
	//<Selector-Detect> 할당 -> 필요 자식노드들 '순서대로' 삽입
	sel_detect->AddChild(sel_canseeplayer);
	sel_detect->AddChild(seq_hasshouting);
	sel_detect->AddChild(seq_hasfootsound);
	sel_detect->AddChild(seq_hordeaction);
	sel_detect->AddChild(seq_hasinvestigated);
	sel_detect->AddChild(t_lookaround);
	sel_detect->AddChild(seq_nothaslastknownplayerlocation);

	//<Selector-CanSeePlayer> 할당 -> 필요 자식노드들 '순서대로' 삽입
	sel_canseeplayer->AddChild(seq_canattack);
	sel_canseeplayer->AddChild(seq_cannotattack);

	//{Sequence-CanAttack} 할당 -> 필요 자식노드들 '순서대로' 삽입
	seq_canattack->AddChild(t_attack);

	//{Sequence-CanNotAttack} 할당 -> 필요 자식노드들 '순서대로' 삽입
	seq_cannotattack->AddChild(t_moveto);

	//{Sequence-HasShouting} 할당 -> 필요 자식노드들 '순서대로' 삽입
	seq_hasshouting->AddChild(t_moveto);

	//{Sequence-HasFootSound} 할당 -> 필요 자식노드들 '순서대로' 삽입
	seq_hasfootsound->AddChild(t_moveto);

	//{Sequence-HordeAction} 할당 -> 필요 자식노드들 '순서대로' 삽입
	seq_hordeaction->AddChild(t_moveto);

	//{Sequence-HasInvestigated} 할당 -> 필요 자식노드들 '순서대로' 삽입
	seq_hasinvestigated->AddChild(t_moveto);

	//{Sequence-NotHasLastKnownPlayerLocation} 할당 -> 필요 자식노드들 '순서대로' 삽입
	seq_nothaslastknownplayerlocation->AddChild(t_moveto);
}

ZombieBT::~ZombieBT()
{
	if (sel_detect) {	// 이미 해제된 메모리 다시 delete하지 않도록 예외처리
		delete(sel_detect);
		sel_detect = nullptr;	// nullptr로 초기화 (나중에 new로 다시 사용 가능)
	}
	if (sel_canseeplayer) {
		delete(sel_canseeplayer);
		sel_canseeplayer = nullptr;
	}
	if (seq_hasshouting) {
		delete(seq_hasshouting);
		seq_hasshouting = nullptr;
	}
	if (seq_hasfootsound) {
		delete(seq_hasfootsound);
		seq_hasfootsound = nullptr;
	}
	if (seq_hordeaction) {
		delete(seq_hordeaction);
		seq_hordeaction = nullptr;
	}
	if (seq_hasinvestigated) {
		delete(seq_hasinvestigated);
		seq_hasinvestigated = nullptr;
	}
	if (seq_nothaslastknownplayerlocation) {
		delete(seq_nothaslastknownplayerlocation);
		seq_nothaslastknownplayerlocation = nullptr;
	}

	if (seq_cannotattack) {
		delete(seq_cannotattack);
		seq_cannotattack = nullptr;
	}
	if (seq_canattack) {
		delete(seq_canattack);
		seq_canattack = nullptr;
	}

	if (t_attack) {
		delete(t_attack);
		t_attack = nullptr;
	}
	if (t_moveto) {
		delete(t_moveto);
		t_moveto = nullptr;
	}
}