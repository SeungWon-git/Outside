# 🧟‍♂️ 멀티 좀비 서바이벌 프로젝트 (**Outside** - Unreal Engine 5)

## 🎮 개요
 - 게임 장르: 멀티 좀비 서바이벌
 - 플랫폼: PC
 - 플레이 인원: 1~4인
 - 개발팀원: 3인 (클라이언트 2명, 서버 1명)
 - 개발기간: 약 1년 (24.07 ~ 25.06)
 - 주요 게임 **feature**:
   > + **좀비 절단 시스템 (N개 절단)**
   > + **좀비 부활 시스템 (재결합)**
   > + **Behavior Tree를 활용한 좀비 AI**
   > + **중앙 집중형 서버 구조**
 - 개발에 사용된 기술 스택:
   + Unreal Engine 5.5, C++, Blueprints, Niagara VFX
   + iocp server, Google protobuf, Behavior Tree
   + GitHub
 - 시연 영상: [▶️ 유튜브 영상 보기](https://www.youtube.com/watch?v=11JJCJvbKck)
<img src="http://img.youtube.com/vi/11JJCJvbKck/0.jpg" width="400">

## 🧑‍💻 맡은 역할
- **좀비 AI 구현** (BT / 서버-클라 동기화 포함)
  - **현실적인 좀비처럼 느껴지도록** 탐색(거리에 따른 랜덤 확률 포착), 주위 살피기, 떠돌기, 휴식하기, 도망치기 로직 설계
  - Custom Behavior Tree + Selector/Sequence/Decorator/Task 확장과 유지보수에 용이하도록 **클래스로 관리**
- HUD/UI 및 나이아가라 이펙트 제작 (특히 **피 이펙트는 자체제작**)
- 전반적인 모든 클라이언트 작업 수행
  > + 좀비, 캐릭터별 특성 및 능력 설계, 구현
  > + 최적화 작업
  > + 로비, 캐릭터 선택창 개선
  > + 인벤토리 시스템 개선
  > + 게임 사운드
  > + 패쓰파인더에 사용되는 커스텀 네비매쉬 생성 편집기 구현
  > + 좀비 슬라이스 초기 작업 → 스켈레탈 메시 절단 및 물리 적용
  > + 그 외 다수
- 버그 추적 및 수정의 대부분을 **주도적으로 수행** / **실질적 팀장 역할** (3인 팀, 기여도 50% 이상 자부!)

---

## 🧩 마주했던 문제 & 해결

### 🔸 Custom Behavior Tree 구현
- 사실적인 좀비 AI를 만들고자 AI 구현 모델을 ***Behavior Tree***로 정하였고 이를 직접 커스텀 서버 코드에서 돌려야 하므로 Selector/Sequence/Decorator 등을 모두 직접 구현하여야 했다!
![BT_Diagram](https://github.com/SeungWon-git/Outside/blob/main/JungSeungWon/%EA%B8%B0%ED%83%80/CustomZombie_BT(UPDATED).png)
- **함수 오버라이딩**을 사용하여 Selector의 Decorator를 구현! → 알아보기도 쉽고 유지관리 및 Selector에 더 다양한 새로운 Task를 붙이기에도 용이함!
- 한 예로 Detect Selector의 자식 노드 중 하나인 CanSeePlayer의 Decorater는 다음과 같이 'bool Detect(Zombie& zom) override' 로 구현되어 있다.
- 실제 구현 코드: 
 [BT의 기반 Task 클래스](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/Task.h), 
 [Selector](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/Selector.h), 
 [트리 할당](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/ZombieBT.cpp#L61), 
 [CanSeePlayer의 Detect Decorator 구현](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/CanSeePlayer.h#L11)
- 이를 통해 **좀비 BT를 서버에서 전적으로 담당**하게 되어 클라이언트에서는 서버에서 전송해주는 Task들(애니메이션 전환, 렌더링, 물리 처리)만 수행하여 모든 클라이언트들에서 좀비 AI 동기화를 수행

### 🔸 좀비 Path 동기화 이슈
- 문제 1: path 동기화 설계
  + 원인 1-1: 모든 경로 전송(* 문제점: 현재 좀비가 경로 상 어느 위치에 있는지도 추가로 알려줘야함)
  + 해결 1-1: **앞선 두 노드만 전송**으로 최소화 & 서버상 현재 위치도 전송하여 정확한 좀비 위치 동기화
  + 원인 1-2: 전체 좀비 BT 한 싸이클이 0.05초 하지만 좀비 위치를 0.05초 마다 서버→클라로 갱신한다 하면 **좀비 뚝뚝 끊김 현상 발생**
  + 해결 1-2: 미리 받은 앞선 두 정점을 가지고 클라에서도 자체적으로 좀비 이동시켜 좀비 움직임 부드럽게 **보간**
- 문제 2: 패킷 손실 이슈
  + 원인: 여러 송신 스레드에서 Send → **Data Race 발생**
  + 해결: 송신 데이터들을 모아 **lock-free 송신큐**에 담는 방식으로 전환 및 단일 송신 스레드 설계
- 문제 3: 좀비 멈춤 현상
  + 원인: for루프를 돌며 너무 반복적이고 연속적으로 패킷을 Send하여 패킷로스가 발생
  + 해결: 층별 묶음 전송 방식으로 개선
- 코드: 
[서버 좀비 path 송신](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/iocpServerClass.cpp#L816), 
[서버 좀비 움직임 계산](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/Zombie.cpp#L636), 
[클라 좀비 움직임 계산](https://github.com/SeungWon-git/Outside/blob/main/unreal/Project/Source/Project/Private/ProZombie/ZombieAIController.cpp#L36)

### 🔸 서버-클라 좀비 플레이어 시야에 포착
- 먼저, 플레이어 시야 포착(CanSeePlayer Task)은 Ray Casting 방식을 이용하여 실제로 플레이어를 포착 했을때 좀비가 플레이어를 쫒아가도록 설계
  - 문제: 다만 문제는 외부 원격 커스텀 서버에서 맵, 장애물, 좀비, 플레이어 메시 정보들을 관리하여 Ray Casting을 직접 수행하기는 **사실상 불가능**!
  - 해결: 따라서 **Ray Casting은 클라에서 직접 실행**하되, 시야에 포착하였다고 **서버로 알려주는 방식** 채용
    + 실제 포착했는지 여부는 또 **서버에서 거리에 따라 랜덤 확률에 따라 판단하여 클라에 전달**
- 코드:
[클라 Ray Casting 검사](https://github.com/SeungWon-git/Outside/blob/main/unreal/Project/Source/Project/Private/ProZombie/ZombieAIController.cpp#L359), 
[서버 거리에 따른 플레이어 랜덤 포착](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/Zombie.cpp#L1511)

### 🔸 스켈레탈 메시 절단 시스템
- 아이디어:
  - 절단 당시 해당 좀비의 포즈에 스켈레탈 메시 정보들을 가지고 **본을 움직여 스켈레탈 메시 → 프로시져 메시로 전환**
  - 프로시져 메시는 언리얼에서 내장 함수'SliceProceduralMesh'로 절단 기능이 이미 존재해 이를 이용
  - 다만 새로 생성된 프로시져 메시는 물리에셋이 없기에 따로 메시 정보를 가지고 물리적용에 필요한 컨벡스 메시를 생성
  - 이때 모든 정점을 순환하게되면 시간이 너무 오래 걸리므로 100/1 간격으로 정점을 돌아서 **비슷하게 물리 에셋을 생성**
  - 이후에 N개 절단(모든 사지 분리 절단) & 절단 부위 결합되며 부활은 다른 클라 담당 팀원이 전적으로 맡았지만 기본 N개 절단 아이디어는 같이 구상
    + 절단면에 버텍스 정보들과 절단 당시 본들의 **정보를 대조**하여 분리되어야 한다고 판단되는 버텍스들을 파악하고, 해당 절단면을 기준으로 떨어져야 하는 **메시들을 따로 묶어** 다시 프로시져 메시로 생성하는 방식으로 구현
  + 이때 절단면을 구분하기 위해 모든 버텍스 정보들을 순회하기에는 시간이 오래 걸리니 **밀도 기반 클러스터링(DBSCAN) 알고리즘을 이용**
    + 알고리즘 사용 후 처리 속도: **0.5초-5초 → 0.01초-0.02초**
 - 코드:
[스켈레탈 메시 → 프로시져 메시 전환하는 함수](https://github.com/SeungWon-git/Outside/blob/main/unreal/Project/Source/Project/Private/ProZombie/BaseZombie.cpp#L1283), 
[N개 절단 함수](https://github.com/SeungWon-git/Outside/blob/main/unreal/Project/Source/Project/Private/ProZombie/BaseZombie.cpp#L1405)

### 🔸 최적화 작업
- 클라이언트:
  * 로컬 클라이언트가 존재하는 층에 있는 좀비들만 렌더/물리/틱 연산 처리 적용
    - 성능: **(최저 권장 사양에서) 20-40 fps → 60-80 fps**
- 서버:
  * BT 최적화: 플레이어가 있는 층에 좀비만 BT 실행
  * A* 알고리즘: 정점 기반 경로 → 선분 기반으로 변경
    - 처리 속도: **0.5초 → 0.001초(=1ms)**
  * 송신큐 **lock-free**로 접근 (***CAS(Compare And Swap) 연산*** 이용) → 낮은 지연, 높은 처리량
 - 코드:
[A* 알고리즘](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/ZombiePathfinder.cpp#L200), [SendQueue Lock-Free](https://github.com/SeungWon-git/Outside/blob/d0069c056e4d1d7483763fe51fab081240fd8236/Server/Game%20Server/Server/iocpServerClass.cpp#L414)

### 🔸 좀비 애니메이션 문제
- 문제: 좀비 공격/피격 후 다시 다음 애니메이션으로 전환될 시에 걷기 애니메이션이 항상 강제적으로 재생이 되어 움찔거리게 되는 문제 발생
- 원인: 클라에서 서버로부터 공격/피격이 끝나고 다음 Task를 받기 전 애니메이션 시퀀스가 끝남에 따라 자동적으로 걷기 State로 돌아가서 발생한 문제임을 파악
- 해결: 공격/피격이 끝나고 약간의 딜레이(idle State 실행)를 주어 애니메이션 전환이 매끄럽도록 수정 
- 코드:
[좀비 애니메이션 전환 딜레이](https://github.com/SeungWon-git/Outside/blob/main/unreal/Project/Source/Project/Private/ProZombie/ZombieAIController.cpp#L53)

### 🔸 플레이어 이동 지연
- 문제: 저사양 환경에서 위치 동기화 딜레이 발생
- 원인: 수신처리 담당 클래스의 Tick에 위치 동기화 패킷 큐에서 틱마다 하나씩 패킷을 뽑아서 동기화시키기 때문에 생기는 문제임을 확인 → 저사양 컴퓨터의 경우 Tick이 느리게 작동 → 패킷이 큐에 계속 쌓임
- 해결:
  + 큐에 일정량 이상 플레이어 이동 패킷이 쌓이면 한번에 뽑아내도록 하는 방식으로 해결
  + 플레이어 위치 이동 패킷 전송도 Tick에서 움직임이 있을때마다 보내지 않고 60분에 1초 단위마다 보내도록 한계 설정
  + 패킷으로 받은 플레이어 위치를 원격 클라에서 직접 적용할 때도 보간을 이용하여 부드럽게 움직이는 것 처럼 보이도록 함
- 코드:
[플레이어 위치 동기화](https://github.com/SeungWon-git/Outside/blob/main/unreal/Project/Source/Project/Private/ProCharacter/PlayerCharacterController.cpp#L128)

### 🔸 게임 시간 측정 오류
- 문제: 게임 시간 10분보다 훨씬 더 빨리 게임이 끝났다고 계산하는 문제 발생
- 원인: 시간을 측정할 때 +deltaTime을 해주는 부분에서 **부동 소수점 오류**가 계속 누적되어 원래 시간보다 더 빨리 시간을 측정하고 있었음
- 해결: 부동 소수점 오류를 피하고자 deltaTime가 **5ms** 이상 일 경우에만 게임 시간에 누적시킴!
- 코드:
[게임 시간 측정](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/iocpServerClass.cpp#L553)

---

## 🐞 버그 관리 전략
- `버그.txt` 문서 작성 → **1000줄 이상 추적** + 영상까지 기록
  + [버그 리스트](https://github.com/SeungWon-git/Outside/blob/main/JungSeungWon/%EA%B8%B0%ED%83%80/%EB%B2%84%EA%B7%B8.txt)
  + 영상 기록
  + <img src="https://github.com/SeungWon-git/Outside/blob/main/JungSeungWon/%EA%B8%B0%ED%83%80/%ED%99%94%EB%A9%B4%20%EC%BA%A1%EC%B2%98%202025-06-21%20181037.png" width="300">
- GitHub + 매주 3회 이상 대면회의 + 주마다 작업일지 병행
  + [주차별 작업일지](https://github.com/SeungWon-git/Outside/tree/SW/JungSeungWon/%EC%9E%91%EC%97%85%EC%9D%BC%EC%A7%80)
- `#ifdef DEBUG` 와 같이 세분화된 전처리기 조건부 로그로 서버 BT 문제 위치 빠르게 추적
  + [전처리기 디렉티브](https://github.com/SeungWon-git/Outside/blob/main/Server/Game%20Server/Server/iocpServerClass.h#L43)

---

## 🌱 이번 프로젝트를 통해 배운점
- 우리는 멀티 협동 좀비 서바이벌 게임을 개발하기 위해, ***정밀한 좀비 AI 동기화***와 ***좀비 절단 표현의 차별화를 특별히 신경***을 써서 만들었다!
   * 이 과정에서 **클라-서버 간 동기화에서 유의해야 하는 점** 등에대해 몸소 경험하였으며, **효율적이고 안정적인 동기화 방식**이 어떤 것인지에 대해 실제로 고민하고 적용해 볼 수 있었다.
   * **AI를 직접 설계**하며 행동 로직 구성의 난이도와 그에 따른 문제를 해결하기 위해 어떤 전략과 기법이 필요한지 깊이 고민하고 시도해보는 계기가 되었습니다.
   * 비록 프로덕션 수준에는 다소 못 미쳤지만, 그에 근접하기 위해 **다양한 언리얼 기능들을 활용**해 보았고, **게임 개발의 전반적인 흐름**과 **협업 시 중요하게 고려해야 할 점들**에 대해 많은 것을 배우게 되었다!

---

## 📎 기타
- 소스 코드: 👉 [서버 소스 코드 폴더](https://github.com/SeungWon-git/Outside/tree/main/Server/Game%20Server/Server), [언리얼-클라 소스 코드 폴더](https://github.com/SeungWon-git/Outside/tree/main/unreal/Project/Source/Project)
- 최종 보고서:  📃 [최종 보고서(한글파일)](https://github.com/SeungWon-git/Outside/blob/main/%EB%B3%B4%EA%B3%A0%EC%84%9C/2025_%EA%B2%8C%EC%9E%84%EA%B3%B5%ED%95%99%EA%B3%BC_%EC%B5%9C%EC%A2%85_%EB%B3%B4%EA%B3%A0%EC%84%9C_%EC%96%91%EC%8B%9D%20-%20%EC%95%84%EC%9B%83%EC%82%AC%EC%9D%B4%EB%93%9C_%EC%A0%95%EC%8A%B9%EC%9B%90%EC%A1%B0%EC%83%81%EC%A4%80%EC%9D%B4%ED%95%9C%EC%A3%BC.hwp)
- Contributors Graphs: [🔗 View Contributors Graph](https://github.com/SeungWon-git/Outside/graphs/contributors)
