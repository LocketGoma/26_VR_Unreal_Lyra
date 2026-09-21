# 택배 분류 샘플 — UE 5.8 / Lyra

VRSimulate의 1인칭 택배 분류 샘플 사용 설명서다. 전체 구성은 `README.md`, 컴포넌트와 입력 연결은 `Docs/ParcelSortingArchitecture.md`를 참고한다.

기본 수치는 C++ 코드 기준이다. 실제 게임에 적용되는 값은 선택된 BP와 배치 인스턴스 설정에 따라 달라진다. 파일 경로는 Feature 루트 기준이다.

## 1. 코드 레벨에서 수정하는 항목

| 변경 목적 | 파일·함수 |
|---|---|
| 작업장·컨베이어 배치와 크기 | `ParcelSortingArena.cpp` → `BuildWorkstation` |
| 택배 생성 위치·분류 확률 | `ParcelSortingArena.cpp` → `SpawnPackage` |
| 컨베이어 이동·끝 지점 판정 | `ParcelSortingArena.cpp` → `Tick` |
| 택배·대차 생성 클래스 | Arena의 `SpawnPackage`, `BuildWorkstation` |
| 적재 범위·출발 판정 | `ParcelCart.cpp` → `GetLoad`, `RequestDeparture` |
| 문·대차 이동 방식 | `ParcelCart.cpp` → `Tick` |
| 점수 계산식·통계 항목 | `ParcelSortingTypes.h`, Arena·Cart 집계 코드 |
| 기본 HUD 배치·문구 | `ParcelSortingGameMode.cpp` → `AParcelSortingHUD` |

헤더는 `Source/VRSimulateRuntime/Public/ParcelSorting`, 구현은 `Source/VRSimulateRuntime/Private/ParcelSorting`에 있다.

GameMode는 사용할 Arena와 Pawn/HUD를 선택한다. Arena는 작업장·택배·대차 생성과 라운드를 관리한다. 택배와 대차는 현재 네이티브 클래스로 생성하므로 해당 BP를 생성에 사용하려면 코드 연결이 필요하다.

## 2. BP 기본값에서 수정하는 항목

### GameMode와 Arena

1. 맵의 World Settings → GameMode Override에 사용할 `ParcelSortingGameMode` 파생 BP를 지정한다.
2. GameMode BP → Class Defaults → Parcel → Arena Class에 사용할 Arena BP를 지정한다.
3. 같은 GameMode의 Default Pawn Class에 사용할 Character BP를 지정한다.
4. Arena BP → Class Defaults → Parcel → Settings에서 라운드 설정을 수정한다.

`Use Placed Arena=false`이면 ArenaClass와 같은 클래스의 배치 Arena를 사용하거나 해당 클래스를 생성한다. 배치 Arena의 개별 설정을 우선하려면 `Use Placed Arena`를 켠다. 교체한 Arena는 선택한 BP의 기본값을 사용한다.

Arena는 하나만 배치하며 회전 `(0,0,0)`, 스케일 `(1,1,1)`을 유지한다. 클래스 변경은 다음 Play에 적용한다.

### Arena Settings

| 속성 | 기본값 (코드) | 용도·적용 |
|---|---|---|
| `GameDuration` | 300초 (코드) | 라운드 제한 시간. 시작·Restart에 적용 |
| `PackageSpawnInterval` | 3초 | 택배 생성 간격. 첫 생성도 해당 간격 후 시작 |
| `ConveyorSpeed` | 65cm/s | 컨베이어 위 택배 이동 속도 |
| `CartReturnTime` | 30초 | 대차 퇴장 후 복귀 전 대기 |
| `CorrectScore` | +1 | 다음 출발의 올바른 물품 점수 |
| `WrongScore` | -5 | 다음 출발의 잘못된 물품 점수. 음수로 지정 |
| `PackageMinScale` | 0.32 | 새 택배의 최소 균일 스케일 |
| `PackageMaxScale` | 0.55 | 새 택배의 최대 균일 스케일 |

크기는 100cm Cube 기준이며 기본 한 변은 32–55cm다. 스케일의 적용 범위는 0.2–0.8이다.

### Character와 카메라

| 설정 위치 | 속성 | 용도 |
|---|---|---|
| Character BP | `InteractionDistance` | 탐색 거리. 기본 350cm (코드) |
| Character BP | `HoldDistance` | 기본 보유 거리. 기본 200cm (코드) |
| CharacterMovement | `MaxWalkSpeed` | 이동 속도. 기본 430cm/s (코드) |
| Grab 컴포넌트 | `DistanceStep`, `FollowSpeed` | 거리 조절량·물체 추적 속도 |
| CameraComponent → Camera Settings | `Field Of View` | 목표 FOV |

탐색 거리와 기본 보유 거리는 Play 시작 시 적용한다.

FOV는 Character BP의 기존 CameraComponent에서 지정한다. 입력 범위는 5–170도다. Character의 `Camera FOV Offset`은 읽기 전용 자동 계산 결과다.

```text
Camera FOV Offset = 입력 FOV - CameraMode의 기본 FOV
예: 모드 기본값 90, 입력 FOV 110 → Offset +20
```

### 입력 설정

Character BP의 InputRouter에서 `Actions`, `MappingContext`, `MappingPriority`를 지정한다. 활성 Enhanced Input 매핑을 우선하고, 매핑이 없는 명령만 기본 키를 사용한다. `bEnableBuiltInFallback=false`이면 기본 키 입력을 끈다.

Action 타입과 IMC 예시는 `Docs/ParcelSortingArchitecture.md`의 입력 설정을 참고한다.

## 3. BP 커스텀으로 수정하는 항목

| 목적 | 사용할 API·이벤트 |
|---|---|
| 활성 Arena 조회 | `Get Arena → Is Valid` 또는 GameMode의 `ActiveArena` |
| 시간·진행 상태 조회 | `Get Remaining Time`, `Is Round Active` |
| 점수·결과 조회 | Arena의 `Stats` |
| 진행 중 게임 시간 변경 | `Set Game Duration` |
| 다음 라운드 설정 변경 | Arena의 `Settings` |
| 라운드 종료·재시작 | `End Round`, `Restart Round` |
| 종료·재시작 연출 연결 | `OnRoundEnded`, `OnRoundRestarted` |
| 대차 출발 요청 | Cart의 `Request Departure` |
| 플레이 중 FOV 변경 | CameraComponent의 `Set Field Of View` |

`Set Game Duration`의 Apply to Current Round가 true이면 경과 시간을 유지하며 총 제한 시간을 바꾼다. 20초 경과 시 60초로 변경하면 40초가 남는다. `Settings.GameDuration`을 직접 바꾸면 다음 Restart에 적용한다.

Widget BP에서 시간·Stats를 표시하고 Restart 버튼에 `Restart Round`를 연결할 수 있다. 새 UI를 사용할 때는 GameMode의 HUD Class를 해당 HUD로 지정한다.

상호작용 대상·피드백 확장은 `Docs/ParcelSortingArchitecture.md`의 BP 커스텀 항목을 참고한다.

## 4. 그 외: 맵·조작·현재 판정 규칙

### 맵 생성

1. 프로젝트를 빌드하고 에디터에서 연다. Windows 실행 절차는 `Docs/ManualEditorBuild.md`를 참고한다.
2. Python Editor Script Plugin을 활성화하고 현재 맵을 저장한다.
3. VRSimulate가 Registered 이상인지 확인하고 Show Plugin Content를 켠다.
4. Tools → Execute Python Script에서 `Scripts/create_parcel_sorting_map.py`를 실행한다.
5. 생성된 `/VRSimulate/Maps/L_ParcelSorting`의 GameMode Override와 Arena 설정을 지정한다.
6. Number of Players=1, Net Mode=Play Standalone으로 Play한다.

기존 맵은 덮어쓰지 않는다. 프로젝트를 다른 폴더로 복제한 경우 스크립트의 `EXPECTED_PROJECT`를 실제 프로젝트 경로로 지정한다.

스크립트는 맵·GameMode 지정·Arena·PlayerStart를 준비한다. 작업장·컨베이어·대차·조명은 Play에서 생성된다.

수동으로 구성할 경우 빈 레벨에 GameMode Override를 지정하고 Arena를 하나 배치하거나 GameMode 생성에 맡긴다. PlayerStart의 기본 위치는 `(0,0,100)`이다.

### 기본 조작

| 입력 | 동작 |
|---|---|
| WASD | 이동 |
| 마우스 | 시점 회전 |
| E 또는 왼쪽 클릭 | 택배 잡기·놓기·대차 출발 버튼 |
| 휠 | 보유 거리 조절 |
| 결과 화면 Restart 또는 R | 재시작 |

Enhanced Input 매핑이 있으면 HUD에 표시되는 현재 키를 사용한다. 택배를 보유한 상태의 Interact는 놓기 동작이다. 대차를 출발시키려면 물품을 놓은 뒤 대차 옆 버튼을 누른다.

선택한 택배는 하늘색, 보유 중인 택배는 금색 테두리로 표시한다. 보유 중인 택배는 컨베이어 이동에서 제외되며, 놓은 뒤에는 위치에 따라 컨베이어 이동 또는 물리 동작이 적용된다. 기본 시선 기준 컨베이어는 오른쪽에서 왼쪽으로 이동한다.

### 현재 판정 규칙

| 조건 | 처리 |
|---|---|
| 택배 분류 | A/B/C/D 중 동일 확률로 생성 |
| 대차 적재·회수 | 점수·Loaded 변화 없음. 출발 전 수정 가능 |
| 출발 수락 | 내부 물품을 일괄 판정 |
| 올바른 물품 / 잘못된 물품 | 기본 +1점 / -5점 (코드) |
| Loaded | 출발 시 판정한 전체 물품 수 |
| 빈 대차 출발 | 허용. 수락된 출발마다 Sent Carts +1 |
| 보유 중·경계에 걸침·이미 처리됨 | 판정 제외 |
| 컨베이어 끝에서 제거 | Missed +1, 점수 변화 없음 |
| 라운드 종료 | 생성·상호작용·신규 출발·집계 중단 |
| 미출발 물품 | 종료 점수에서 제외 |
| Restart | 통계·택배·대차 위치/문/상태·타이머·생성·캐릭터 위치/입력 초기화 |

기본 게임 시간은 **300초 (코드)**다. 실제 제한 시간은 Arena Settings를 따른다. 결과 화면에는 총점, 보낸 대차 수, 실은 물품 수, 잘못 실은 물품 수, 놓친 물품 수를 표시한다. HUD의 `ready`는 출발 전 판정 가능한 물품 수이며 누적 Loaded와 구분한다.

대차는 출발 판정 → 문 닫힘 → 퇴장 → 대기 → 빈 대차 복귀 순서로 동작한다. 코드 기본값은 문 닫힘 0.8초, 편도 이동 2초, 퇴장 후 대기 30초다. Restart는 현재 Settings로 라운드를 시작한다.

### UE 5.8 공식 문서

BP 지정자와 기본값 설정은 다음 문서를 참고한다.

| 참고 항목 | 공식 문서 |
|---|---|
| `BlueprintCallable`, `BlueprintNativeEvent` 등 | [Exposing Gameplay Elements to Blueprints](https://dev.epicgames.com/documentation/en-us/unreal-engine/exposing-gameplay-elements-to-blueprints-visual-scripting-in-unreal-engine?application_version=5.8) |
| Class Defaults | [Blueprint Editor Defaults Tab](https://dev.epicgames.com/documentation/unreal-engine/blueprint-editor-defaults-tab?application_version=5.8) |
