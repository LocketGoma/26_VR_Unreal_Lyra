# 택배 분류 샘플 — UE 5.8 / Lyra

확인한 프로젝트: `F:\Programing\Git\26_VR\LyraStarterGame\LyraStarterGame.uproject`.

작업 범위: `F:\Programing\Git\26_VR\LyraStarterGame\Plugins\GameFeatures\VRSimulate` 내부만.
Unreal MCP를 사용하지 않고 C++ 코드와 사용자가 실행할 맵 생성용 Python 스크립트로 구성했다.

## 현재 상태

2026-09-12 컴포넌트 리팩토링: 기존 Base/Interaction/Grab/Feedback 구조를 활용해 Character의 대상 탐색·Grab·테두리를 분리하고 Enhanced Input 우선/fallback 입력을 추가했다. 상세 역할·설정·수업 순서·수동 확인은 [리팩토링 수업 가이드](ParcelSortingArchitecture.md)를 참고한다. **이번 리팩토링도 빌드·UHT·에디터·자동화 테스트를 실행하지 않았다.** 아래 이관 점검 기록은 이전 작업 기록이다.

이번 이관 점검에서는 파일·모듈 참조 점검 및 로컬 Lyra/UE 5.8 엔진 소스와의 API 대조만 수행했다. **빌드, UHT, 에디터 실행, PIE, Unreal 자동화 테스트, 맵 생성 스크립트의 Unreal 실행을 모두 하지 않았다.** 실행 성공을 보장하거나 검증 완료로 표시하지 않는다.

완료한 정적 검사: 맵 스크립트의 Python AST, 플러그인 JSON, C++ 파일 12개의 샘플 include 경로·export 매크로·generated header 순서, 이전 루트의 중복 소스 부재, WASD 축 바인딩 제거, `git diff --check`. 모두 통과했다. 이는 Unreal 실행 검증과 별개다.

프로젝트 `.uproject`, LyraGame 소스, 프로젝트 전역 설정, 엔진 소스는 수정하지 않았다. `Content/Maps/L_SampleMap.umap`을 포함한 기존 바이너리 에셋은 보존했으며 새 `.umap`이나 Blueprint 에셋은 생성하지 않았다. 기존 맵의 역직렬화·참조 정상 여부는 에디터에서 확인해야 한다. `L_SampleMap`을 이 샘플의 완성 맵으로 간주하지 않는다.

## 이관 수정 및 보완

- 루트에 옮겨졌던 코드를 `Source/VRSimulateRuntime/Public/ParcelSorting`과 `Private/ParcelSorting`으로 정리했다.
- 샘플 클래스·구조체 export 매크로를 `VRSIMULATERUNTIME_API`로 수정했다. include는 `ParcelSorting/...`을 사용한다.
- Build.cs에 LyraGame, GAS 관련 모듈, InputCore, PhysicsCore 의존성을 명시했다. 플러그인에 GameplayAbilities 의존성을 추가했다. 기존 ShooterCore/ShooterTests/LyraExampleContent 설정은 유지했다.
- 생성 스크립트의 클래스 경로를 `/Script/VRSimulateRuntime.*`, 맵 경로를 `/VRSimulate/Maps/L_ParcelSorting`으로 수정했다. 현재 프로젝트와 스크립트 위치가 위 경로와 일치해야 실행된다.
- `Config/DefaultVRSimulate.ini`에 이전 `/Script/LyraGame.Parcel...` 클래스·구조체·열거형의 정확한 리다이렉트를 추가했다. 이전 에셋을 복구하는 데 필요한 연결 정보이며 바이너리 에셋을 자동 수정하거나 이동하지 않는다.
- 택배 크기를 물리 등록 전에 적용하고 컨베이어의 장식 선 충돌을 제거했다. 큰 프레임 간격으로 컨베이어 끝을 건너뛰어도 이전 접촉 정보를 이용해 Missed를 기록한다.
- 적재 판정은 실제 메시의 회전된 꼭짓점을 검사한다. E와 클릭이 같은 프레임에 들어왔을 때 중복 상호작용을 방지한다.
- 이전 이관에서 WASD의 잘못된 `BindAxisKey` 사용을 제거했다. 현재 입력은 InputRouter가 동작별 Enhanced Input 우선순위를 처리하고, 활성 매핑이 없는 동작에만 기본 키 polling을 적용한다.
- 종료 시 캐릭터 이동 모드를 정지하고, Restart에서 이동·시점·Grab 거리·GAS 입력 상태와 대차의 원래 Transform을 복원한다.

## 실행 준비

사용자가 추후 `LyraEditor / Development Editor / Win64` 빌드를 수행한 후 에디터를 다시 연다. 새 UCLASS가 추가되었으므로 처음에는 에디터를 닫은 상태에서 일반 빌드하는 편이 명확하다. 아래 스크립트는 빌드를 실행하지 않는다.

자동 맵 생성:

1. 에디터에서 **Python Editor Script Plugin**을 활성화한다. 필요한 경우 에디터를 재시작한다.
2. 현재 작업 중인 맵을 먼저 저장한다.
3. Content Browser의 **Show Plugin Content**를 켜고 VRSimulate Game Feature가 **Registered 이상**인지 확인한다. 기존 Feature 상태 설정은 변경하지 않았다.
4. **Tools → Execute Python Script**에서 `F:\Programing\Git\26_VR\LyraStarterGame\Plugins\GameFeatures\VRSimulate\Scripts\create_parcel_sorting_map.py`를 실행한다.
5. `/VRSimulate/Maps/L_ParcelSorting` 맵이 생성된다. 디스크 또는 에셋 레지스트리에 이미 존재하면 덮어쓰지 않는다. 다른 프로젝트에서는 레벨 생성 전에 중단한다.
6. 맵의 `Parcel Sorting - Settings` 액터를 선택하여 Settings를 조절한다.
7. **Number of Players = 1, Net Mode = Play Standalone**으로 Selected Viewport에서 Play한다. 작업장·컨베이어·대차·조명은 BeginPlay에서 생성된다. Simulate 모드가 아닌 Play 모드를 사용한다.

Python을 사용하지 않는 수동 설정:

1. 빈 기본 레벨을 만들어 `/VRSimulate/Maps/L_ParcelSorting`에 저장한다.
2. World Settings의 GameMode Override를 `ParcelSortingGameMode`로 지정한다.
3. `ParcelSortingArena`를 원점 `(0,0,0)`, 회전 `(0,0,0)`, 스케일 `(1,1,1)`로 배치한다. 하나만 사용한다.
4. PlayerStart를 `(0,0,100)`, 회전 `(0,0,0)`에 배치하고 Play한다.

Arena나 PlayerStart가 없으면 게임 모드가 기본 인스턴스를 생성한다. 설정을 바꾸려면 Arena를 레벨에 직접 배치한다. Arena는 하나만 사용하고 회전 0, 스케일 1을 유지한다. 원점 이동은 가능하다. 기존 맵이 있어서 스크립트가 건너뛰었다면 위 수동 설정으로 GameMode Override와 Arena를 점검한다. 이전 `/Game/ParcelSorting` 맵 또는 Shooter 테스트 맵은 이 스크립트가 변환하지 않는다.

클래스를 찾지 못하면 생성 스크립트를 반복 실행하기 전에 해당 프로젝트를 빌드했는지, VRSimulateRuntime 모듈과 Feature가 로드되었는지 확인한다. 이전 에셋의 부모 클래스 복구와 리다이렉트 적용 여부도 사용자가 에디터에서 확인하고 저장해야 한다. 이 Feature의 기존 Shipping 제외 설정은 유지하므로 이 문서는 Development Editor 샘플 실행을 대상으로 한다.

## 조작

### 택배 선택 표시

- 조준점으로 가리킨 상호작용 거리 내 택배는 **하늘색 발광 테두리**로 표시한다.
- 손에 든 택배는 **금색 발광 테두리**를 유지한다. 들고 있는 동안에는 다른 택배를 강조하지 않는다.
- 시선을 옮기거나 거리를 벗어나면 이전 강조가 해제된다. 출발 처리, 라운드 종료, Restart, 빙의 해제 때에도 정리한다.
- A/B/C/D 문자와 분류 색상은 유지한다. 테두리 색상은 조준/Grab 상태를 뜻한다.

상자의 12개 모서리에 얇은 Cube를 붙이는 C++ 방식이다. 상자의 회전·크기를 따라가며, 가려진 부분을 벽 너머로 보여주는 효과는 아니다. 첫 조준 시에만 테두리 컴포넌트를 생성한다. 충돌·Overlap·그림자·내비게이션은 끄고, 적재 판정은 기존 Box 메시만 사용한다.

기존 맵에서 사용 가능하며 **맵 생성 스크립트 재실행, Custom Depth 설정, Post Process Volume 또는 새 머티리얼 에셋 생성이 필요 없다.** 사용자가 C++를 빌드한 후 실행하면 적용된다. 엔진의 `DebugMeshMaterial`을 사용하며, 두께와 하늘색/금색 및 발광 강도는 `UVRSimOutlineFeedbackComponent`의 SelectedColor/HeldColor/EmissiveIntensity/EdgeThickness 설정에서 조절할 수 있다.

선택 표시 변경은 소스 정적 점검만 수행했으며 빌드·에디터 실행은 하지 않았다. PIE에서는 택배 간 조준 전환, Grab/Release 색상 전환, 장애물 뒤/거리 밖 해제, 종료/Restart 해제와 테두리 밝기를 확인한다.

아래는 Enhanced Input 매핑이 없는 경우의 기본 키다. 사용자 매핑이 있으면 HUD에 표시되는 현재 키를 사용한다.

| 입력 | 동작 |
|---|---|
| WASD | 이동 |
| 마우스 | 시점 회전 |
| E 또는 왼쪽 클릭 | 바라보는 택배 잡기 / 들고 있는 택배 놓기 / 출발 버튼 누르기 |
| 휠 | 들고 있는 택배의 거리 조절 |
| 결과 화면 Restart 클릭 또는 R | 새 라운드 |

기본 시선에서 컨베이어는 오른쪽(+Y)에서 왼쪽(-Y)으로 흐른다. 컨베이어 양 끝을 돌아 대차 쪽 통로로 이동할 수 있다. 대차 전면 옆의 작은 큐브가 출발 버튼이다. 택배를 들고 있을 때 Interact는 놓기로 동작하므로 놓은 뒤 버튼을 누른다.

택배 이동은 충돌을 검사하는 위치 이동이며, 놓으면 물리가 재개된다. 손에 든 동안 벨트 이동에서는 제외된다. 벨트에 다시 놓으면 벨트 이동이 재개되고 바닥에 놓으면 물리적으로 떨어진다. 쌓기와 충돌 감각은 실제 PIE에서 추가 조정이 필요하다.

## 규칙

- 택배는 동일 확률로 A/B/C/D를 선택하고, 기본 0.32–0.55 배율로 생성한다.
- 적재 및 다시 꺼내기에는 점수·Loaded 통계를 기록하지 않는다.
- 출발 버튼을 누를 때 Overlap 결과를 갱신해 내부 물품의 스냅샷을 만든다.
- 물품 전체가 적재 공간 안에 들어 있어야 한다. 경계에 걸친 물품, 손에 든 물품, 이전에 처리된 물품은 제외한다. HUD의 대차별 **ready** 숫자가 현재 판정 대상 수다. 최종 **Loaded packages** 통계와 구분한다. 판정 경계에는 물리 접촉 오차를 위한 1cm 허용치를 둔다.
- 스냅샷의 올바른 물품은 +1, 잘못된 물품은 -5로 판정한다. 예시 A/A/C/A → Score -2, Loaded 4, Wrong 1.
- 빈 대차 출발도 Sent Carts를 1 증가시킨다. 출발 요청이 수락되어 판정되는 순간 기록을 확정한다.
- 출발 문은 평소 숨겨져 있고 충돌이 없다. 출발 시 위쪽에 나타나 내려온 뒤 대차가 뒤쪽으로 이동한다.
- 대차가 작업 영역을 벗어나면 내부 물품을 제거한다. 기본 30초 대기한 다음 빈 대차가 복귀한다. 문 닫힘 0.8초와 왕복 이동 각 2초는 대기 시간과 별도다.
- 기본 라운드는 300초다. 종료 시 샘플의 생성·이동·상호작용·신규 출발·집계를 중단하고 택배 물리와 대차 연출을 정지한다. 아직 출발하지 않은 물품은 점수에 포함하지 않는다. 출발이 이미 수락된 물품은 문 닫힘·이동 중 종료되어도 결과에 포함된다.
- 벨트 끝에서 제거되는 물품만 Missed에 기록하며 점수 패널티는 없다. 바닥에 떨어뜨린 물품은 다시 주울 수 있다.
- Restart는 모든 택배, 대차 위치·문·상태, 통계, 타이머, 플레이어 위치와 입력 상태를 초기화한다.

## 에디터 조절 값

Arena → Settings에서 GameDuration, PackageSpawnInterval, ConveyorSpeed, CartReturnTime, CorrectScore, WrongScore, PackageMinScale, PackageMaxScale을 바꿀 수 있다. 시간·간격·크기는 런타임에도 최소값을 적용한다. GameDuration 변경은 다음 Restart부터 적용된다.

| 값 | 기본값 | 적용 |
|---|---:|---|
| GameDuration | 300초 | 다음 라운드/Restart부터 |
| PackageSpawnInterval | 3초 | 첫 생성도 이 간격 후, 이후 생성 간격 |
| ConveyorSpeed | 65 cm/s | 벨트 위의 놓인 택배에 적용 |
| CartReturnTime | 30초 | 퇴장 완료 후 복귀 전 대기 |
| CorrectScore | +1 | 다음 출발 시 |
| WrongScore | -5 | 다음 출발 시, 패널티는 음수로 입력 |
| PackageMinScale | 0.32 | 새 택배 최소 균일 배율 |
| PackageMaxScale | 0.55 | 새 택배 최대 균일 배율 |

크기는 0.2–0.8 범위로 제한하고 최대값이 최소값보다 작으면 최소값을 사용한다. 100cm Cube 기준 기본 한 변은 32–55cm다. 생성 지점이 막혀 있으면 해당 시도를 건너뛸 수 있으며 Missed로 세지 않는다. 프레임 지연으로 밀린 생성을 한꺼번에 몰아서 만들지 않는다.

## 코드 구조

| 파일 | 역할 |
|---|---|
| `Source/VRSimulateRuntime/Public/ParcelSorting/ParcelSortingTypes.h` | 분류, 대차 상태, 설정, 통계 |
| `ParcelPackage.h/.cpp` | 물리 상자, 네 면 문자, 상호작용/잡기 인터페이스, 처리 완료 잠금 |
| `ParcelCart.h/.cpp` | 내부 Overlap, 출발 시 스냅샷 집계, 문·퇴장·복귀 상태 |
| `ParcelSortingArena.h/.cpp` | 코드 기반 작업장, 생성·컨베이어, 라운드·통계·초기화 |
| `ParcelSortingCharacter.h/.cpp` | LyraCharacter 기반 1인칭 캐릭터 및 GAS Interaction Ability |
| `ParcelSortingGameMode.h/.cpp` | 샘플 진입, Canvas HUD 및 결과 Restart 버튼 |

표에서 축약한 `.h`는 `Source/VRSimulateRuntime/Public/ParcelSorting`, `.cpp`는 `Source/VRSimulateRuntime/Private/ParcelSorting` 기준이다.

샘플 전용 `AGameModeBase`를 사용하며 Lyra의 Experience 선택 과정은 거치지 않는다. 캐릭터는 `ALyraCharacter`, ASC는 `ULyraAbilitySystemComponent`, 상호작용은 `ULyraGameplayAbility`를 재사용한다. 싱글플레이 샘플이므로 ASC는 캐릭터가 소유한다. Lyra PlayerState ASC / PawnData / HeroComponent / ShooterCore 상호작용 스캔에 연결된 완전한 Experience 구성은 아니다. 입력은 InputRouter에 지정한 Input Action을 Lyra의 EnhancedInputComponent에 바인딩한다. 현재 활성 매핑이 없는 동작에는 기본 키 fallback을 제공한다.

UI는 외부 Widget 에셋 없이 Canvas로 구현했고 기본 폰트 가독성을 위해 영문으로 표시한다. UI 교체용으로 Arena의 Blueprint 접근 가능한 통계·남은 시간과 OnRoundEnded / OnRoundRestarted 이벤트를 제공한다.

## 검증 항목 — 미실행

사용자 요청에 따라 택배 샘플의 자동화 테스트 소스를 제거했다. 아래 항목은 사용자가 직접 확인할 수동 검증 절차다.

PIE에서 추가 확인할 항목:

1. 1인칭 마우스/키 입력, GAS 활성화, 상자 Grab 후 놓기와 다시 잡기.
2. 벨트 이동 및 끝에서 Missed만 증가, 벨트에 다시 놓았을 때 이동 재개.
3. 상자 쌓기, 경계에 걸친 상자 제외, 손에 든 상자 제외, 벽 충돌 시 Grab 감각.
4. Settings.GameDuration을 10으로 두고 시간 종료·물리 정지·재시작 버튼 클릭 확인.
5. 이동 중이거나 Away 상태인 대차가 있는 채로 라운드를 종료하고 Restart.
6. 화면 해상도를 바꾸며 HUD와 결과 버튼 가독성 확인.
