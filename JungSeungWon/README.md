# 🧟‍♂️ 멀티 좀비 서바이벌 프로젝트 (**Outside** - Unreal Engine 5)

## 개요


## 🧑‍💻 맡은 역할
- 좀비 AI 구현 (BT / 서버-클라 동기화 포함)
  - 실제 좀비처럼 느껴지도록 탐색(거리에 따른 랜덤 확률 포착), 주위 살피기, 떠돌기, 휴식하기, 도망치기 로직 설계
  - Custom Behavior Tree + Selector/Sequence/Decorator/Task 확장과 유지보수에 용이하도록 클래스로 관리
- HUD/UI 및 나이아가라 이펙트 제작 (특히 피 이펙트는 자체제작)
- 대다수의 클라이언트 작업 수행
  - 좀비, 캐릭터별 특성 및 능력 설계, 구현
  - 최적화 작업
  - 로비, 캐릭터 선택창 리워크
  - 인벤토리 시스템 리워크
  - 게임 사운드
  - 패쓰파인더에 사용되는 커스텀 네비매쉬 생성 편집기 구현
  - 좀비 슬라이스 초기 작업 → 스켈레탈 메시 절단 및 물리 적용
  - 그 외 다수
- 버그 추적 및 수정의 대부분을 주도적으로 수행 버그 수정 / 실질적 팀장 역할 (3인 팀, 기여도 50% 이상 자부!)

---

## 🧩 마주했던 문제 & 해결

### 🔸 Custom Behavior Tree 구현
 - 사실적인 좀비 AI를 만들고자 AI 구현 모델을 Behavior Tree로 정하였고 이를 직접 커스텀 서버 코드에서 돌려야 하므로 Selector/Sequence/Decorator 등을 모두 직접 구현하여야 했다!
 ![BT_Diagram](https://github.com/2023gamedev/project/blob/SW/JungSeungWon/%EA%B8%B0%ED%83%80/CustomZombie_BT(UPDATED).png)
 - 함수 오버라이딩을 사용하여 Selector의 Decorator를 구현! → 알아보기도 쉽고 유지관리 및 Selector에 더 다양한 새로운 Task를 붙이기에도 용이함!
 - 한 예로 Detect Selector의 자식 노드 중 하나인 CanSeePlayer의 Decorater는 다음과 같이 'bool Detect(Zombie& zom) override' 로 구현되어 있다.
 - 실제 구현 코드: 
 [BT의 기반 Task 클래스](https://github.com/2023gamedev/project/blob/SW/Server/Game%20Server/Server/Task.h), 
 [Selector](https://github.com/2023gamedev/project/blob/SW/Server/Game%20Server/Server/Selector.h), 
 [트리 할당](https://github.com/2023gamedev/project/blob/SW/Server/Game%20Server/Server/ZombieBT.cpp), 
 [CanSeePlayer 구현](https://github.com/2023gamedev/project/blob/SW/Server/Game%20Server/Server/CanSeePlayer.h)

### 🔸 최적화 작업
- 클라이언트:
  * 플레이어 층에만 렌더/물리/틱 연산 처리 적용
    - 성능 **(최저 권장 사양에서) 20-40 fps → 60-80 fps**
- 서버:
  * BT 최적화: 플레이어가 있는 층에 좀비만 BT 실행
  * A* 알고리즘: 정점 기반 경로 → 선분 기반으로 변경 
    - 처리 속도 **0.5초 → 0.0005초(=0.5ms)**

### 🔸 좀비 Path 동기화 이슈
- path 동기화 설계: 모든 경로 전송(*문제: 현재 좀비가 경로 상 어느 위치에 있는지도 추가로 알려줘야함) → 앞선 두 정점만 전송으로 최소화 & 서버상 현재 위치도 전송하여 정확한 좀비 위치 동기화
  (좀비 BT 전체 싸이클이 0.05초 하지만 좀비 위치를 0.05초 마다 갱신한다 하면 좀비 뚝뚝 끊김 현상 발생 → 미리 받은 앞선 두 정점을 가지고 클라에서도 자체적으로 좀비 이동시켜 좀비 움직임 부드럽게 보간)
- 패킷 손실: 큐 방식 전환 및 단일 송신 스레드 설계
- 좀비 멈춤 현상: for루프를 돌며 너무 반복적이고 연속적으로 패킷을 Send하여 패킷로스가 발생 → 층별 묶음 전송 방식으로 개선

### 🔸 애니메이션 문제
- 공격/피격 후 애니 전환 딜레이 없이 걷기 → idle 상태 삽입하여 해결

### 🔸 플레이어 이동 지연
- 저사양 환경에서 위치 동기화 딜레이 → 큐 버퍼링 조건 추가
- 위치 패킷 전송을 60Hz 제한 + 클라에서 보간 적용

### 🔸 서버-클라 인식 불일치
- CanSeePlayer 상태 동기화 안됨 → BT/클라 모두 시야 상태 초기화 적용

### 🔸 스켈레탈 메시 절단 시스템
- 본 정보를 기준으로 메시 절단 후 Procedural Mesh로 전환
- 언리얼 내장 Slice 기능 사용
- 물리 에셋 미포함 문제 → 정점 일부 샘플링하여 Convex 생성

---

## 🐞 버그 관리 전략
- `버그.txt` 문서 작성 → **1000줄 이상 추적**, 영상까지 기록
- GitHub + 주 3회 회의 + 작업일지 병행
- `#ifdef DEBUG` 조건부 로그로 서버 BT 문제 위치 빠르게 추적

---

## 🧰 기술 스택
- Unreal Engine 5.5, C++, Blueprints
- Niagara VFX, Behavior Tree
- GitHub

---

## 📎 기타
- GitHub 저장소: [링크]
- 시연 영상: [YouTube 링크]
