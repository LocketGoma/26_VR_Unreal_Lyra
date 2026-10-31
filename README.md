# VRSimulate — Lyra 기반 택배 분류 샘플

Unreal Engine 5.8의 Lyra Starter Game을 기반으로 구성한 싱글플레이 1인칭 택배 분류 샘플이다. 이동·카메라·GAS(Gameplay Ability System)는 Lyra 기능을 재사용하고, 상호작용·잡기·선택 표시를 컴포넌트로 분리한다. 게임 규칙과 작업장 생성은 `ParcelSorting`에 둔다.

이 문서의 기본 수치는 C++ 코드 기준이며, 실제 게임에 적용되는 값은 선택된 BP와 인스턴스 설정에 따라 달라진다. UE 기본 개념과 BP 지정자는 문서 하단의 UE 5.8 공식 문서를 참고한다.

| 항목 | 내용 |
|---|---|
| 프로젝트 | `LyraStarterGame.uproject` |
| Feature 및 코드 수정 범위 | 프로젝트 내 `Plugins/GameFeatures/VRSimulate` |
| 런타임 모듈 | `VRSimulateRuntime` |
| 샘플 맵 에셋 경로 | `/VRSimulate/Maps/L_ParcelSorting` |
| 기본 게임 시간 (코드) | 300초 |
| 기본 입력 | 키보드·마우스, Enhanced Input 우선 |
| 화면 구성 | Cube 기반 작업장·택배·대차, Canvas 기반 HUD |
| 대상 외 기능 | 멀티플레이, 별도 네트워크 구현, AI, 복잡한 인벤토리, 차량 물리, VR HMD 입력, 서버·저장, 패키징 |

## 1. 코드 레벨에서 수정해야 하는 항목

### 1.1 소스 구조와 책임

아래 경로는 Feature 루트 기준이다. 공개 선언은 `Source/VRSimulateRuntime/Public`, 구현은 `Source/VRSimulateRuntime/Private`에 있으며 하위 폴더 구성이 대응한다.

```text
VRSimulate/
├─ Source/VRSimulateRuntime/
│  ├─ Public/ 및 Private/
│  │  ├─ Base/           공간 기준점, 입력 라우팅
│  │  ├─ Interaction/    대상 탐색, 선택, 상호작용 인터페이스
│  │  ├─ Grab/           잡기·놓기, 이동·물리 상태 관리
│  │  ├─ Feedback/       선택·보유 상태 표시
│  │  ├─ ParcelSorting/  샘플 게임 규칙, 작업장, 카메라 모드, HUD
│  │  ├─ Teleport/       별도 확장용 코드
│  │  ├─ Tools/          별도 도구 동작 연결용 코드
│  │  └─ Example/        상호작용 예제 코드
│  └─ VRSimulateRuntime.Build.cs
├─ Content/              맵·BP 등 Unreal 에셋
├─ Config/               Feature 설정과 이전 클래스 경로 리다이렉트
├─ Scripts/              맵 생성·Windows 빌드 스크립트
├─ Docs/                 세부 사용 설명서
├─ BuildAndOpenLyraEditor.bat
└─ VRSimulate.uplugin
```

| 클래스·구조체 | 책임 | 주요 진입점 |
|---|---|---|
| `AParcelSortingGameMode` | Pawn/HUD 선택, 활성 Arena 선택·생성 | `StartPlay`, `InitializeArena` |
| `AParcelSortingArena` | 작업장·대차 생성, 택배 생성, 컨베이어, 라운드·통계 | `BuildWorkstation`, `SpawnPackage`, `Tick`, `RestartRound`, `EndRound` |
| `AParcelSortingCharacter` | LyraCharacter 상속, 컴포넌트 구성, 입력 전달, 카메라 연결 | `BeginPlay`, `HandleInputCommand`, `PerformInteraction`, `CalcCamera` |
| `UParcelCameraMode` | Lyra 기반 1인칭 카메라 모드 | Character의 카메라 연결 |
| `UParcelInteractionAbility` | Grab/Release/대차 버튼의 GAS 진입점 | `CanActivateAbility`, `ActivateAbility` |
| `UVRSimInputRouterComponent` | Enhanced Input과 기본 키의 우선순위·중복 처리 | `SetupInput`, `PollFallbackInput`, `OnCommand` |
| `UVRSimControllerProxyComponent` | 탐색·잡기에 사용하는 공간 기준점 | 시점과 동기화한 Transform |
| `UVRSimRayInteractionComponent` | LineTrace 탐색 | `UpdateDetection` |
| `UVRSimInteractionComponent` | 선택 상태, Hit 결과, 상호작용 전달 | `SetSelectedHit`, `TryInteract` |
| `UVRSimGrabComponent` | 보유 대상, Sweep 이동, 물리 상태 복원 | `TryGrab`, `TryRelease`, `UpdateManipulation` |
| `UVRSimOutlineFeedbackComponent` | 선택·보유 테두리, 동적 머티리얼 인스턴스 | `PlaySelectionFeedback`, `SetHeldFeedback` |
| `AParcelPackage` | 분류, 잡기 인터페이스, 처리 완료 상태 | `Initialize`, `SealForDispatch` |
| `AParcelCart` | 적재 검사, 출발 판정, 문·퇴장·복귀 | `GetLoad`, `RequestDeparture`, `Tick`, `ResetCart` |
| `FParcelSortingSettings` / `FParcelSortingStats` | 조절 값 / 누적 통계 | `ParcelSortingTypes.h` |
| `AParcelSortingHUD` | 시간·점수·결과 표시, Restart 클릭 | `DrawHUD`, `NotifyHitBoxClick` |

`UParcelInteractionAbility`는 Character 헤더·구현 파일에, `AParcelSortingHUD`는 GameMode 헤더·구현 파일에 포함되어 있다. Arena는 작업장과 라운드를 관리하는 Actor이며, 택배 생성 책임은 GameMode가 아니라 Arena에 있다.

### 1.2 실행 흐름

1. GameMode가 `ArenaClass`와 배치된 Arena를 검사하여 `ActiveArena`를 결정한다.
2. 선택된 Arena가 `BeginPlay`에서 작업장·대차·조명을 만들고 라운드를 시작한다.
3. Character가 입력·상호작용·Grab 컴포넌트와 자체 ASC(Ability System Component)를 연결한다.
4. Character Tick에서 시점 기준점 갱신 → 대상 탐색 → 보유 물체 이동 순서로 처리한다.
5. 상호작용 입력은 아래 경로로 전달된다.

```text
InputRouter.Interact
  → ASC.TryActivateAbilityByClass
  → UParcelInteractionAbility
  → Character.PerformInteraction
      ├─ 보유 중: Grab.TryRelease
      └─ 미보유: Interaction.TryInteract
                    ├─ 택배: Grab.TryGrab
                    └─ 대차 버튼: RequestDeparture
```

공통 Interaction/Grab 컴포넌트는 택배 분류 규칙을 참조하지 않는다. Character가 탐색과 보유 물체 이동 순서를 관리한다. 선택한 택배는 하늘색, 보유 중인 택배는 금색 테두리로 표시한다.

### 1.3 C++ 변경 위치

| 변경 목적 | 주요 파일·함수 | 현재 제약 |
|---|---|---|
| 작업장·컨베이어 크기, 배치, 조명 | `ParcelSortingArena.cpp` → `BuildWorkstation` | 치수·좌표가 코드에 지정됨 |
| 택배 생성 위치·분류 확률·생성 방식 | `ParcelSortingArena.cpp` → `SpawnPackage` | A/B/C/D 균등 분류, 범위 내 랜덤 크기 |
| 컨베이어 접촉·끝 지점 판정 | `ParcelSortingArena.cpp` → `Tick` | 정해진 작업장 좌표·높이 기준 |
| 택배·대차를 BP 클래스로 생성 | `SpawnPackage`, `BuildWorkstation` | 현재 `AParcelPackage`, `AParcelCart`를 직접 생성하며 클래스 선택 속성 없음 |
| 분류 종류 추가 | `ParcelSortingTypes.h` 및 생성·대차 배치 코드 | enum만 늘려서는 생성 개수·랜덤 범위가 변경되지 않음 |
| 적재 범위·출발 조건·중복 집계 규칙 | `ParcelCart.cpp` → `GetLoad`, `RequestDeparture` | 규칙 함수는 BP 오버라이드 이벤트가 아님 |
| 문·대차 이동 방식 | `ParcelCart.cpp` → `Tick` | Transform 기반 상태 전환, 차량 물리 미사용 |
| 점수 계산식·통계 항목 | `ParcelSortingTypes.h`, Arena·Cart 집계 코드 | 점수 계수만 바꾸는 경우 BP Settings 사용 가능 |
| 기본 Canvas HUD 배치·문구·표시 항목 | `ParcelSortingGameMode.cpp` → HUD 구현 | 기존 화면은 UMG Widget 에셋이 아님 |
| 새 입력 명령·GAS 동작 추가 | `VRSimInputRouterComponent`, Character·Ability | 기존 명령의 키 변경은 Enhanced Input 설정으로 처리 |

택배·대차 BP의 기본값을 바꾸더라도 현재 Arena의 생성 클래스는 자동 교체되지 않는다. 생성 클래스 노출이 필요한 경우 Feature 내부 C++ 변경이 선행되어야 한다.

### 1.4 현재 판정 규칙

| 시점·조건 | 처리 |
|---|---|
| 대차에 물품을 넣음 | 점수·Loaded 증가 없음. 출발 전 회수·수정 가능 |
| 출발 요청 수락 | 현재 물품 목록을 한 번 수집하여 일괄 판정 |
| 올바른 물품 / 잘못된 물품 | 기본 +1점 / -5점 |
| Loaded | 출발 시 판정한 전체 물품 수. 정답·오답 모두 포함 |
| 빈 대차 출발 | 허용. 수락된 출발마다 Sent Carts +1 |
| 보유 중·이미 처리된 물품 | 적재 판정 제외 |
| 적재 경계에 걸친 물품 | 물품 전체가 적재 범위에 포함되어야 함 |
| 컨베이어 끝에서 제거 | Missed +1, 점수 변화 없음 |
| 작업장 아래로 떨어져 정리됨 | Missed 증가 없음 |
| 라운드 종료 | 생성·상호작용·신규 출발·집계 중단. 미출발 물품은 결과 제외 |
| Restart | 통계·택배·대차 위치/문/상태·시간·생성 대기·캐릭터 위치/입력 초기화 |

대차는 출발 판정 → 문 닫힘 → 퇴장 → 대기 → 복귀 순서로 동작한다. 점수는 출발 판정 시 확정된다. 코드 기본값은 문 닫힘 0.8초, 편도 이동 2초, 퇴장 후 대기 30초다.

라운드 종료 시 택배 물리와 대차 연출도 정지한다. Restart는 라운드 상태를 초기화하며 BP/Settings를 최초 기본값으로 되돌리는 기능은 아니다. 현재 Settings로 다음 라운드를 시작한다.

### 1.5 Lyra 재사용 범위

샘플 GameMode는 `AGameModeBase` 기반이다. Character는 `ALyraCharacter`, ASC는 `ULyraAbilitySystemComponent`, Ability는 `ULyraGameplayAbility`를 사용한다. ASC는 Character가 소유한다.

코드 수정은 VRSimulate 내부로 제한한다. LyraGame·엔진 소스의 기존 기능을 복제하거나 직접 수정하지 않는다. 새 주석은 영어로 작성하고 클래스 책임·단위·전제조건 등 정보를 기술한다.

## 2. BP 기본값에서 수정할 수 있는 항목

### 2.1 적용 경로와 우선순위

```text
맵 World Settings → GameMode Override
  └─ ParcelSortingGameMode 파생 BP
       ├─ Default Pawn Class → ParcelSortingCharacter 파생 BP
       ├─ HUD Class → 사용할 HUD 클래스
       └─ Arena Class → ParcelSortingArena 파생 BP
```

### 2.2 Arena 선택

GameMode BP → Class Defaults → Parcel에서 설정한다.

| 속성 | 기본값 | 적용 |
|---|---|---|
| `ArenaClass` | `ParcelSortingArena` | Play에 사용할 Arena 클래스 |
| `Use Placed Arena` | false | true이면 맵에 배치된 Arena를 클래스와 무관하게 우선 |
| `ActiveArena` | 실행 중 결정 | 선택 결과 조회 전용 |

`Use Placed Arena=false`이면 ArenaClass와 같은 클래스의 배치 Arena를 사용하거나 해당 클래스를 생성한다. 교체한 Arena는 선택한 BP의 기본값을 사용한다. 배치 Arena의 개별 설정을 우선하려면 `Use Placed Arena`를 켠다.

Arena는 하나만 배치하며 회전 `(0,0,0)`, 스케일 `(1,1,1)`을 유지한다. Arena 클래스 변경은 다음 Play에 적용한다.

### 2.3 Arena Settings

Arena BP → Class Defaults → Parcel → Settings에서 수정한다. 선택된 배치 Arena의 Details에서도 인스턴스 값을 지정할 수 있다.

| 속성 | 기본값·단위 | 적용 시점 및 범위 |
|---|---|---|
| `GameDuration` | 300초 (코드) | 라운드 시작·Restart. BP·인스턴스 설정으로 변경 가능. 실행 중 단순 대입은 현재 종료 시각을 변경하지 않음 |
| `PackageSpawnInterval` | 3초 | 생성 후 다음 대기 설정 시. 현재 남은 대기 시간은 즉시 초기화하지 않음. 최소 0.1초 |
| `ConveyorSpeed` | 65cm/s | 진행 중 컨베이어 갱신 시. 최소 1 |
| `CartReturnTime` | 30초 | 대차 Away 상태의 대기 판정 시. 최소 0 |
| `CorrectScore` | +1 | 다음 출발 판정 |
| `WrongScore` | -5 | 다음 출발 판정. 음수 값을 직접 지정 |
| `PackageMinScale` | 0.32배 | 새 택배 생성 시. 0.2–0.8 범위 |
| `PackageMaxScale` | 0.55배 | 새 택배 생성 시. 최소 크기 이상, 최대 0.8 |

크기는 100cm Cube의 균일 스케일이다. 기본 한 변은 32–55cm다. 첫 택배도 라운드 시작 직후가 아닌 생성 간격이 지난 후 생성한다. 생성 위치가 막혀 생성을 건너뛴 경우 Missed로 기록하지 않는다.

### 2.4 Character와 Grab

| 설정 위치·속성 | 기본값 | 적용 및 제약 |
|---|---|---|
| Character → `InteractionDistance` | 350cm | BeginPlay에서 Ray의 TraceDistance에 적용 |
| Character → `HoldDistance` | 200cm | BeginPlay에서 Grab의 DefaultHoldDistance에 적용 |
| CharacterMovement → `MaxWalkSpeed` | 430cm/s | 기존 이동 컴포넌트의 속도 설정 |
| Grab → `MinHoldDistance` | 90cm | 최소 보유 거리 |
| Grab → `DistanceStep` | 20cm/입력 단위 | 거리 조절 입력의 변화량 |
| Grab → `FollowSpeed` | 14 | 목표 위치 보간 속도 |
| Interaction → `TraceChannel` | Visibility | 탐색에 사용할 충돌 채널 |
| Interaction → `Debug Interaction` | false | 탐색 디버그 표시 |

탐색 거리와 기본 보유 거리는 Character의 `InteractionDistance`, `HoldDistance`에서 설정한다. 두 값은 Play 시작 시 적용된다.

### 2.5 카메라 FOV

목표 FOV는 **Character BP → Components → CameraComponent → Camera Settings → Field Of View**에서 지정한다. Character의 `Camera FOV Offset`은 플레이 중 계산 결과를 조회하는 읽기 전용 속성이다.

```text
Camera FOV Offset = CameraComponent에 입력한 목표 FOV
                    - 선택된 CameraMode 클래스의 기본 FieldOfView

모드 기본값 90, 목표 FOV 110 → Offset +20
모드 기본값 80, 목표 FOV 110 → Offset +30
```

입력 범위는 5–170도다. 플레이 중 CameraComponent의 `Set Field Of View`로 변경할 수 있으며, 다음 카메라 갱신에서 Offset이 자동 계산된다.

### 2.6 Enhanced Input

Character BP의 InputRouter 컴포넌트에서 `Actions`, `MappingContext`, `MappingPriority`를 지정한다. `bEnableBuiltInFallback` 기본값은 true다.

| 명령 | Input Action 값 타입 | 값 계약 | 기본 키 |
|---|---|---|---|
| Move | Axis2D | X=오른쪽, Y=앞쪽 | WASD |
| Look | Axis2D | X=Yaw, Y=Pitch | 마우스 이동 |
| Interact | Boolean | 누름당 한 번 | E 또는 왼쪽 클릭 |
| AdjustDistance | Axis1D | 양수=멀리, 음수=가까이 | 마우스 휠 |
| Reset | Boolean | 종료 화면에서 재시작 | R |

해당 명령에 활성 Enhanced Input 매핑이 있으면 그 매핑을 우선 사용한다. 매핑이 없는 명령은 기본 키를 사용한다. `bEnableBuiltInFallback=false`이면 기본 키 입력을 끈다.

마우스 Y 반전·감도는 IMC Modifier에서 설정한다. Boolean 동작은 누른 동안 Triggered가 발생하는 설정을 사용하며 Released Trigger는 현재 계약에 포함하지 않는다. 상세 축 매핑 예시는 별도 컴포넌트 가이드에 있다.

## 3. BP 커스텀으로 수정할 수 있는 항목

### 3.1 공개 API를 이용한 라운드·설정 제어

| 목적 | BP 경로·API | 계약 |
|---|---|---|
| 활성 Arena 참조 | `Get Arena → Is Valid` | GameMode가 실제 선택한 Arena 조회 |
| 현재 점수·통계 표시 | Arena → `Stats` | 읽기 전용. 집계는 출발 규칙에서 수행 |
| 남은 시간·진행 상태 | `Get Remaining Time`, `Is Round Active` | UI 조회용 |
| 진행 중 제한 시간 변경 | `Set Game Duration` | Apply to Current Round=true이면 경과 시간을 유지하며 종료 시각 변경 |
| 다음 라운드 설정 변경 | `Settings` 수정 또는 Apply to Current Round=false | GameDuration은 다음 Restart에 적용 |
| 설정 구조체 변경 | Settings Get → Set Members → Settings에 다시 대입 | Arena의 Settings에 변경값 반영 |
| 종료·재시작 | `End Round`, `Restart Round` | 기존 정리·초기화 경로 사용 |
| 결과 화면·라운드 시작 연출 | `OnRoundEnded`, `OnRoundRestarted` 바인딩 | 종료·재시작 시 UI와 연출 갱신 |
| 대차 출발 요청 | Cart → `Request Departure` | 반환값으로 수락 여부 확인. 점수 로직은 기존 경로 유지 |
| 런타임 FOV 변경 | CameraComponent → `Set Field Of View` | 다음 Character 카메라 평가에서 목표값 반영 |

`Set Game Duration`은 라운드 시작으로부터의 총 제한 시간을 바꾼다. 예를 들어 20초 경과 시 60으로 설정하면 40초가 남는다. 경과 시간 이하로 줄이면 종료되며 이미 종료된 라운드는 자동 재개하지 않는다.

현재 Arena는 `Get Arena` 또는 GameMode의 `ActiveArena`로 조회한다.

### 3.2 새 상호작용·잡기 대상

일반 BP Actor에 아래 인터페이스를 구현하여 공통 컴포넌트와 연결할 수 있다. 컨베이어·분류·출발 집계는 `AParcelPackage`에 의존하므로 일반 잡기 대상이 자동으로 택배로 집계되는 것은 아니다.

| 인터페이스 | 구현 지점 | 책임 |
|---|---|---|
| `VRSimInteractable` | `CanInteract` | 현재 상호작용 허용 여부 |
| 동일 | `OnSelected`, `OnDeselected` | 선택 상태 표시·해제 |
| 동일 | `Interact` | 허용된 상호작용 수행 |
| `VRSimGrabbable` | `CanGrab` | 보유 가능 여부와 중복 소유 제한 |
| 동일 | `GetGrabPrimitive` | 이동할 Movable 루트 PrimitiveComponent 반환 |
| 동일 | `OnGrabbed`, `OnReleased` | 대상 상태·피드백 갱신 |

GrabComponent가 보유 물체의 이동과 물리 상태를 관리한다.

단순 선택·상호작용 대상에는 `AVRSimInteractableTarget`의 `On Selection Changed`, `On Interaction` 이벤트도 사용할 수 있다. 새 Pawn에서 재사용하려면 Proxy → Interaction → Grab 참조와 갱신 순서를 직접 구성한다.

### 3.3 탐색·피드백 확장

Interaction의 `UpdateDetection`, Grab의 `TryGrab`·`TryRelease`·`UpdateManipulation`, Feedback의 `PlaySelectionFeedback`·`PlayInteractionFeedback`·`PlayDestinationFeedback`은 BlueprintNativeEvent 확장 지점이다.

`BuildWorkstation`, `SpawnPackage`, `GetLoad`는 현재 BP 오버라이드 지점으로 노출되지 않았다. `BlueprintCallable`, `BlueprintNativeEvent` 등의 지정자는 문서 하단의 UE 5.8 공식 문서를 참고한다.

파생 컴포넌트 BP를 만든 것만으로 Character가 C++ 생성자에서 만든 기본 컴포넌트가 교체되지는 않는다. 새 Pawn에서 구성하거나, 기존 Character의 생성·참조 연결을 코드에서 변경해야 한다. 별도 컴포넌트를 추가한 경우 기존 탐색·Grab 갱신과 중복되지 않도록 구성한다.

### 3.4 UI 교체

Widget BP에서 Arena의 시간·Stats를 읽고 라운드 이벤트에 연결하여 UMG 화면을 구성할 수 있다. Restart 버튼은 기존 `Restart Round`를 호출한다. 위젯 생성·Viewport 추가·제거는 선택한 HUD 또는 PlayerController BP에서 구성한다.

GameMode의 `HUD Class`를 새 HUD 클래스로 지정한다.

## 4. 그 외: 실행 준비·참고 자료

### 저사양 PC용 품질·메모리 절약 스크립트

에디터에서 `Python Editor Script Plugin`을 켠 뒤 `Tools → Execute Python Script`로 `Scripts/optimize_low_spec.py`를 실행한다. Effects는 중간(Medium, 1), 나머지 품질 Low, 30 FPS, 렌더 해상도 50%, 텍스처 스트리밍 256 MiB, 고비용 렌더링 비활성화를 설정 파일에 저장한다. Niagara 호환성을 위해 메시 거리 필드는 원본을 보존하며, 실행 중 품질 전환 없이 재시작 후 적용한다. 셰이더 워커는 최대 4개를 목표로 확보한다. 원본을 백업하며 Windows/macOS/Linux 터미널 실행·옵션·복원 방법은 [저사양 설정 가이드](Docs/LowSpecOptimizer.md)를 참고한다.

Windows에서는 에디터를 닫고 `OpenLowSpecLyraEditor.bat`을 실행하면 엔진 내장 Python으로 프로파일을 먼저 적용한 뒤 빈 맵에서 시작한다. `OpenLowSpecLyraEditor.bat -SampleMap`은 택배 분류 맵을 바로 연다. 시작 시 에셋 탭 복원·Live Coding 자동 시작·소스 관리 연결을 줄이며 DDC·셰이더 캐시는 유지한다.

### 4.1 Windows 수동 빌드와 에디터 실행

Feature 루트의 `BuildAndOpenLyraEditor.bat`을 더블클릭한다. Visual Studio C++ 도구와 Windows SDK가 필요하며 현재 프로젝트 에디터를 닫은 상태에서 실행한다.

```text
스크립트 위치에서 상위 .uproject 탐색
  → 엔진 선택
  → LyraEditor / Win64 / Development 빌드
  → 성공 시 에디터 실행
```

엔진 선택 순서는 명시한 EngineRoot/UE_ENGINE_ROOT → 프로젝트 EngineAssociation → 연결 정보가 비어 있는 경우 상위 소스 엔진 → 설치된 최신 런처 엔진 → 경로 수동 입력이다. “최신”은 로컬 설치 중 최신을 뜻하며 다운로드하지 않는다. 엔진 fallback은 소스 호환성을 보장하지 않는다.

```bat
BuildAndOpenLyraEditor.bat -DetectOnly
BuildAndOpenLyraEditor.bat -NoLaunch
```

`-DetectOnly`는 탐색 결과만 표시하고 빌드·에디터 실행을 하지 않는다. `-NoLaunch`는 빌드만 수행한다. 스크립트는 VRSimulate 포함 옵션을 전달하며 패키징·Cook·프로젝트 파일 생성·자동화 테스트를 실행하지 않는다. `.bat`과 `Scripts/Build-LyraEditor.ps1`의 상대 위치를 유지한다.

### 4.2 맵 생성

1. 사용자가 프로젝트를 빌드한 후 에디터에서 해당 프로젝트를 연다.
2. Python Editor Script Plugin을 활성화하고 현재 맵을 저장한다.
3. VRSimulate가 Registered 이상인지 확인하고 Show Plugin Content를 켠다.
4. Tools → Execute Python Script에서 Feature의 `Scripts/create_parcel_sorting_map.py`를 실행한다.
5. 생성된 `/VRSimulate/Maps/L_ParcelSorting`에서 GameMode Override와 Arena 설정을 확인한다.
6. Number of Players=1, Net Mode=Play Standalone으로 Play한다.

스크립트는 프로젝트 경로와 스크립트 위치를 검사한다. 기존 맵이 있으면 덮어쓰거나 갱신하지 않는다. 다른 경로로 프로젝트를 복제한 경우 `EXPECTED_PROJECT`를 실제 `.uproject` 경로로 수정해야 한다.

생성 스크립트는 레벨·GameMode 지정·Arena·PlayerStart를 준비한다. 작업장 Cube·컨베이어·대차·조명은 Play의 Arena 초기화에서 생성되므로 편집 화면의 맵 구성과 실행 화면이 다르다. 기존 맵에 코드 변경을 적용하기 위해 생성 스크립트를 반복 실행할 필요는 없다.

수동 구성 시 빈 레벨의 GameMode Override를 지정하고, 선택할 Arena를 원점·회전 0·스케일 1로 하나 배치하거나 GameMode 생성에 맡긴다. PlayerStart의 기본 위치는 `(0,0,100)`이다. Restart의 캐릭터 위치는 현재 Arena 기준 `(0,0,100)`으로 재설정된다.

### 4.3 확인 순서

| 증상 | 확인 대상 |
|---|---|
| 변경한 Arena BP가 사용되지 않음 | 맵의 GameMode Override, GameMode의 ArenaClass, Use Placed Arena, 배치 인스턴스 기본값 |
| 활성 Arena 확인 필요 | Output Log의 `ParcelSorting:`. 실제 GameMode·요청 클래스·ActiveArena·GameDuration 기록 |
| 편집 화면에 작업장이 없음 | 작업장 생성 시점은 Arena BeginPlay |
| FOV 변경이 반영되지 않음 | GameMode의 Default Pawn Class와 해당 Character BP의 CameraComponent → Field Of View |
| 키 변경 후 기본 키도 동작함 | InputRouter Actions 타입, 현재 적용된 IMC, 실제 생성된 Pawn 확인 |
| 택배·대차 BP 수정이 반영되지 않음 | Arena가 네이티브 클래스를 직접 생성하는지 확인 |
| 적재했는데 Loaded가 증가하지 않음 | 출발 시 확정하는 규칙. `ready`와 Loaded는 다른 값 |

### 4.4 구현 범위

공통 컴포넌트 분리, GAS 상호작용, Enhanced Input 우선순위, 출발 시 집계, 대차 상태 전환, Restart, Arena 선택, FOV 자동 Offset은 현재 소스에 구현되어 있다. Feature 이름과 별개로 이 택배 샘플에는 HMD 입력을 연결하지 않았다. NearInteraction은 현재 근거리 기준점·디버그 표시를 제공하며 완성된 Overlap 탐색 구현으로 간주하지 않는다. Teleport/Tools 코드는 택배 샘플의 기본 실행 흐름에 연결되어 있지 않다.

플러그인 모듈은 Shipping 구성에서 제외되어 있다. 실행 안내는 Development Editor 기준이다.

### 4.5 세부 문서

| 문서 | 내용 |
|---|---|
| `Docs/ParcelSorting.md` | 게임 규칙, 맵 생성, Arena·카메라 설정 |
| `Docs/ParcelSortingArchitecture.md` | 컴포넌트 책임, 호출 순서, 인터페이스, Enhanced Input 매핑 예시 |
| `Docs/ManualEditorBuild.md` | Windows 빌드 스크립트 옵션, 엔진 탐색 |

### UE 5.8 공식 문서

UE 기본 개념과 BP 관련 지정자는 다음 Unreal Engine 5.8 공식 문서를 참고한다.

| 참고 항목 | UE 5.8 공식 문서 |
|---|---|
| `BlueprintCallable`, `BlueprintNativeEvent`, `BlueprintImplementableEvent`, `BlueprintReadOnly`, `BlueprintReadWrite` 등 | [Exposing Gameplay Elements to Blueprints](https://dev.epicgames.com/documentation/en-us/unreal-engine/exposing-gameplay-elements-to-blueprints-visual-scripting-in-unreal-engine?application_version=5.8) |
| Class Defaults, 기본값 조회 | [Blueprint Editor Defaults Tab](https://dev.epicgames.com/documentation/unreal-engine/blueprint-editor-defaults-tab?application_version=5.8) |
| C++·BP 인터페이스 구현 | [Interfaces in Unreal Engine](https://dev.epicgames.com/documentation/unreal-engine/interfaces-in-unreal-engine?application_version=5.8) |
| Input Action, Mapping Context, Modifier, Trigger | [Enhanced Input](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine?application_version=5.8) |
