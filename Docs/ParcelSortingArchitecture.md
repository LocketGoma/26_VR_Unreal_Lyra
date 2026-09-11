# VRSimulate 컴포넌트 리팩토링 수업 가이드

## 범위와 검증 상태

대상 프로젝트: `F:\Programing\Git\26_VR\LyraStarterGame\LyraStarterGame.uproject`.
변경 범위는 `Plugins/GameFeatures/VRSimulate` 내부다.

2026-09-12 리팩토링은 **C++ 구현 및 정적 검토 단계**다. 빌드, UHT, 에디터, PIE, Unreal 자동화 테스트는 실행하지 않았다. 기존 샘플의 이전 실행 결과와 이번 리팩토링의 실행 검증은 구분해야 한다. 바이너리 맵/Blueprint/머티리얼 에셋은 변경하지 않았다.

정적 점검: 실제 프로젝트 경로, 플러그인 JSON, 기존 맵 스크립트의 Python AST, Feature include/export/generated-header 순서, 이전 Character의 직접 Grab/Trace/키 바인딩 참조 제거, 공통 컴포넌트의 Parcel 의존성 부재, 공백·충돌 마커를 확인했다. `git diff --check`도 통과했다. Enhanced Input 바인딩/키 조회와 HitResult API는 로컬 UE 5.8.1 엔진 소스에 대조했다. 정적 점검은 C++ 컴파일 성공을 뜻하지 않는다.

## 수업에서 읽을 순서

1. `ParcelSortingCharacter`: 컴포넌트 생성과 연결, 입력 전달.
2. `VRSimRayInteractionComponent → VRSimInteractionComponent`: Trace 결과, 선택 변경, 상호작용 계약.
3. `VRSimGrabComponent → IVRSimGrabbable → ParcelPackage`: 잡기 상태와 물리 상태의 분리.
4. `VRSimOutlineFeedbackComponent`: 선택/보유 상태를 시각 효과로 표현.
5. `ParcelCart → ParcelSortingArena`: 출발 판정과 라운드 통계.
6. `VRSimInputRouterComponent`: Enhanced Input 우선순위와 기본 키 fallback.

공통 컴포넌트는 `Base / Interaction / Grab / Feedback`에, 샘플 규칙은 `ParcelSorting`에 둔다. 각각 헤더는 `Source/VRSimulateRuntime/Public`, 구현은 `Private`에서 찾는다.

## 클래스 책임

| 클래스 | 책임 | 보관하는 상태 |
|---|---|---|
| ParcelSortingGameMode | 기본 Pawn/HUD 선택, 없는 Arena/PlayerStart 생성 | GameMode 설정 |
| ParcelSortingArena | 작업장 구성, 택배 생성·컨베이어, 라운드와 통계 | 타이머, 택배/대차 목록, 점수 |
| ParcelSortingCharacter | Lyra 이동·시점, 컴포넌트 연결, 입력 전달, 샘플 생명주기 | 구성 컴포넌트와 ASC |
| VRSimControllerProxyComponent | 상호작용의 공간 기준점 | 시점에서 동기화한 Transform과 Offset |
| VRSimInputRouterComponent | 동작별 Enhanced Input/기본 키 선택, 중복 입력 차단 | 자신이 설치한 바인딩/Context, 입력 전환 상태 |
| VRSimInteractionComponent | 선택 대상 관리, CanInteract 확인, 이벤트 전달 | SelectedTarget, 최근 FHitResult |
| VRSimRayInteractionComponent | Visibility LineTrace 및 Debug 표시 | 거리, Trace Channel |
| VRSimGrabComponent | 잡은 대상 관리, 위치 보간과 Sweep, 물리 복원 | GrabbedTarget, 원래 물리 상태, 보유 거리 |
| IVRSimInteractable | 선택/상호작용 대상 계약 | 없음 |
| IVRSimGrabbable | 잡기 허용·대상 Primitive·잡기/놓기 알림 계약 | 없음 |
| VRSimFeedbackComponent | 피드백의 기존 Blueprint 확장 지점 | 사용 여부 |
| VRSimOutlineFeedbackComponent | 선택/보유 테두리, MID 및 모서리 메시 관리 | 선택/보유 표시 상태 |
| ParcelPackage | 분류·문자·처리 완료 상태, 인터페이스 구현 | Destination, Processed, Conveyor 접촉 |
| ParcelCart | 적재 검사, 출발 판정, 문과 이동 상태 | 상태, 원래 위치, 출발 스냅샷 |

택배 생성은 **Arena의 책임**이다. GameMode는 Arena를 준비하는 진입점이다. Arena는 엔진의 기반 Actor 클래스를 대체하는 상속용 Base가 아니라, 이 샘플의 작업장과 라운드를 관리하는 Actor다.

## 실행 흐름

### 조준과 표시

`Character.Tick → 시점 위치/회전을 Proxy에 반영 → Ray.UpdateDetection → SetSelectedHit → CanInteract → OnSelected/OnDeselected → Package.Feedback`

캐릭터와 HUD에 별도 Trace를 두지 않는다. HUD는 Interaction의 최근 Hit/선택을 읽는다. Hit에는 Actor뿐 아니라 **맞은 Component**도 남는다. 대차는 버튼 Component에 맞았을 때만 선택·상호작용을 허용하므로 벽을 보고 출발시키지 않는다.

같은 Actor를 계속 보고 있어도 CanInteract를 다시 검사한다. 다른 코드에서 대차가 출발하거나 택배가 처리 완료되면 이전 선택은 해제된다.

### Grab / Release / 출발

`InputRouter.Interact → ASC.TryActivateAbilityByClass → UParcelInteractionAbility → Character.PerformInteraction`

- 이미 잡은 대상이 있으면 `Grab.TryRelease`.
- 잡은 대상이 없으면 `Interaction.TryInteract`가 현재 시점으로 다시 탐색한다.
- 택배는 `Interact → Grab.TryGrab → IVRSimGrabbable`로 연결한다.
- 대차는 `Interact → RequestDeparture`로 연결한다.
- 공통 Interaction/Grab 컴포넌트는 ParcelPackage, ParcelCart, Arena를 참조하지 않는다.

TryInteract의 true는 인터페이스 호출이 전달되었다는 의미다. 실제 출발 수락 여부와 점수는 Cart/Arena가 결정한다. 인터페이스의 기존 void Interact 계약은 유지한다.

### 잡은 물체 이동

GrabComponent가 물체의 Movable 루트 Primitive를 확인하고 원래 SimulatePhysics 값을 저장한다. 보유 중 물리를 끄고 Proxy 기준 목표 위치로 보간한 뒤 Sweep 이동한다. Release 시 저장한 물리 상태를 복원한다. 물리가 켜지는 경우 선속도·각속도를 0으로 초기화한다.

택배는 Grabber를 약한 참조로 역참조하며 `IsHeld()`에서 GrabComponent의 실제 대상을 확인한다. Character에는 별도의 HeldPackage/HighlightedPackage가 없다. 택배의 보유 여부와 컨베이어/적재 제외 판정이 같은 상태를 읽는다.

샘플에서는 Interaction과 Grab의 자체 Tick을 끄고 Character가 **Proxy 갱신 → 탐색 → 보유 위치 갱신** 순서로 호출한다. 다른 Pawn에 재사용할 때는 이 방식 또는 Tick prerequisite 방식 중 하나만 사용한다. 두 방식으로 중복 호출하지 않는다.

## 유지한 게임 규칙

출발 전 적재에는 점수와 Loaded를 기록하지 않는다. 대차 출발 수락 시에만 전체 스냅샷을 검사한다. 올바른 물품 +1, 잘못된 물품 -5, Loaded는 검사한 총수다. 빈 출발도 Sent Carts가 증가한다. 잡은 물품·경계에 걸친 물품·이미 처리된 물품은 제외한다.

종료 후 출발/생성/집계는 차단하며 미출발 물품은 결과에서 제외한다. Missed는 컨베이어 끝 제거 시에만 증가하고 점수 패널티는 없다. 문 닫힘·퇴장·대기·빈 대차 복귀와 Restart의 전체 초기화 구조는 유지한다.

## Enhanced Input 설정

기존 네이티브 Character/GameMode와 맵은 그대로 사용할 수 있다. Input Action을 지정하지 않으면 기본 키가 동작한다. 리팩토링 적용을 위해 **맵 생성 스크립트를 다시 실행할 필요는 없다**.

사용자가 C++ 빌드를 완료한 후 에디터에서 다음을 설정한다.

1. `ParcelSortingCharacter`를 부모로 Blueprint Character를 만들고 VRSimulate 내부에 저장한다.
2. 아래 표의 Input Action 5개와 Input Mapping Context 하나를 VRSimulate 내부에 생성한다.
3. Blueprint Character의 **InputRouter 컴포넌트 → Actions**에서 명령별 Input Action을 지정한다. 일부 명령만 지정해도 된다.
4. 같은 컴포넌트의 **MappingContext**에 IMC를 지정한다. MappingPriority는 기본 0이며 필요에 따라 조정한다.
5. `ParcelSortingGameMode`를 부모로 Blueprint GameMode를 만들고 Default Pawn Class를 위 Character로 설정한다.
6. 샘플 맵 World Settings의 GameMode Override에 위 GameMode를 지정하고 저장한다.

Lyra의 기본 입력 컴포넌트는 Enhanced Input 계열이다. 별도로 교체한 프로젝트라면 Pawn의 InputComponent가 UEnhancedInputComponent 또는 그 파생형인지 확인한다.

| Command | 권장 Action 이름 | Value Type | 값 계약 |
|---|---|---|---|
| Move | IA_Parcel_Move | Axis2D | X=오른쪽, Y=앞쪽 |
| Look | IA_Parcel_Look | Axis2D | X=Yaw, Y=Pitch 입력 |
| Interact | IA_Parcel_Interact | Boolean | 한 번의 활성 입력으로 Grab/Release/출발 |
| AdjustDistance | IA_Parcel_Reach | Axis1D | 양수=멀리, 음수=가까이 |
| Reset | IA_Parcel_Reset | Boolean | 결과 화면에서 재시작 |

IMC 예시:

| Action | Key | Modifier |
|---|---|---|
| Move | D | 없음 |
| Move | A | Negate X |
| Move | W | Swizzle Input Axis Values: YXZ |
| Move | S | Negate X, 다음 Swizzle YXZ |
| Look | Mouse XY 2D-Axis | Negate Y만 켜기 |
| Interact | E 또는 원하는 키 | 없음 |
| AdjustDistance | Mouse Wheel Axis | 없음 |
| Reset | R 또는 원하는 키 | 없음 |

Look의 마우스 Y 부호 반전은 IMC에서 수행한다. Enhanced 값에 Character가 다시 반전하지 않는다. 감도·반전·키 변경은 Enhanced Input Modifier/Mapping에서 조절한다. 버튼은 기본 Trigger 또는 Pressed/Down/Hold처럼 **눌린 동안 Triggered가 발생하는 설정**을 사용한다. Released Trigger는 이 샘플의 Boolean 누름 계약에 포함하지 않는다. Interact/Reset은 Triggered가 지속되어도 Completed/Canceled 전까지 한 번만 전달한다.

### 우선순위

1. Router에 해당 Action이 정상 타입으로 바인딩되고, 로컬 플레이어의 **현재 적용된 매핑**에 키가 있으면 그 명령은 Enhanced Input만 사용한다.
2. 해당 명령에 활성 매핑이 없으면 기본 키를 사용한다.
3. 기본 키가 다른 활성 Enhanced Action에 사용 중이면 그 키의 fallback도 차단한다. Mouse XY와 X/Y, Wheel Axis와 Scroll Up/Down의 중복도 차단한다.
4. 같은 명령은 한 프레임에 한 번만 전달한다. 결과 화면에서는 Reset만 전달한다.
5. Context 전환/Restart 때 눌려 있던 기본 키는 한 번 놓은 후 재입력해야 한다.

예: Interact를 F로 바꾸면 F만 상호작용하고 기존 E/왼쪽 클릭은 추가로 실행되지 않는다. Move를 설정하지 않았다면 WASD fallback은 유지된다. E를 다른 Enhanced Action에 할당하면 E로 fallback 상호작용이 발생하지 않는다.

우선순위 판단은 IMC 에셋의 원래 키 목록이 아니라 LocalPlayer의 적용된 Action Mapping을 읽는다. 실제 적용된 런타임 remap/Context 변경을 반영한다. Context가 제거된 뒤에는 해당 명령의 fallback이 복귀한다. 변경 사항은 엔진의 Mapping 재구축이 완료된 뒤 반영된다.

Router의 MappingContext는 선택 사항이다. 다른 시스템이 Context를 관리한다면 비워 두고 Actions만 지정한다. Router는 직접 추가한 Context와 바인딩만 제거한다. 이미 외부에서 적용한 Context의 우선순위를 변경하거나 제거하지 않는다. Router가 소유한 Context를 외부 시스템과 공동 소유하지 않는다.

Action 지정이나 MappingContext 속성 자체를 플레이 중 교체했다면 `SetupInput`을 현재 InputComponent로 다시 호출한다. 같은 Action의 실제 키 remap은 재호출 없이 반영한다. `bEnableBuiltInFallback=false`면 기본 키를 모두 끈다. 매핑이 존재하지만 Trigger 조건을 충족하지 못하는 경우에는 기본 키로 우회하지 않는다.

HUD는 현재 적용된 키를 표시한다. 키 이름이 긴 매핑은 작은 창에서 가독성을 수동 확인한다. 재시작 UI 버튼 클릭은 키 바인딩과 별도로 계속 사용할 수 있다.

## 재사용 방법과 변경된 API

기존 Base/Interaction/Grab/Feedback 폴더와 클래스는 유지한다. 새 클래스는 InputRouter, Grabbable 인터페이스, OutlineFeedback만 추가했다. 기존 `VRSimRayInteractionExampleComponent`도 남겨 두고 실제 Trace를 부모 구현에 위임한다.

새 물체에는 IVRSimInteractable과 IVRSimGrabbable을 구현한다. Grabbable의 CanGrab에서 사용 가능 여부와 중복 보유를 검사하고 GetGrabPrimitive에서 **Movable 루트 Primitive**를 반환한다. OnGrabbed/OnReleased에서 대상의 보유 상태와 표시를 갱신한다. 샘플 Package 구현을 참고하되 분류·점수 규칙은 복사할 필요가 없다.

새 Pawn에는 Proxy, RayInteraction, Grab을 연결한다. Proxy는 카메라, 다른 SceneComponent 등 호출자가 정한 기준으로 갱신한다. 공통 컴포넌트는 특정 Character나 HMD를 요구하지 않는다.

| 기존 책임/API | 현재 위치 |
|---|---|
| Character.TraceInteraction / HighlightedPackage | RayInteraction.Trace 및 Interaction.SelectedTarget |
| Character.HeldPackage / 보유 위치 보간 | GrabComponent |
| Package.Grab / Release | GrabComponent.TryGrab / TryRelease |
| Package.CreateHighlightEdges / RefreshHighlight | OutlineFeedbackComponent |
| Package.SetInteractionHighlight | Feedback.PlaySelectionFeedback |
| Character의 직접 BindKey/BindAxisKey/WASD polling | InputRouter |
| GrabComponent.SetGrabbedTarget / ClearGrabbedTarget | 기존 이름 유지, 실제 물리와 알림을 포함하는 획득/해제 API |

직접 C++ 호출했던 Package.Grab/Release는 GrabComponent 호출로 변경한다. 선택된 대상을 잡을 때 TryGrab, 명시적 대상을 잡을 때 SetGrabbedTarget을 사용한다. 기존 Blueprint 확장에서 새 기본 동작을 이용하려면 해당 BlueprintNativeEvent의 부모 구현을 호출한다. 대상의 직접 SetActorLocation/SetSimulatePhysics는 GrabComponent와 동시에 제어하지 않는다.

Character의 기존 InteractionDistance/HoldDistance 속성은 유지하고 BeginPlay에서 컴포넌트 설정에 적용한다. 샘플의 최대 보유 거리는 InteractionDistance - 20cm, 최소 거리 이상이다. 런타임 보유 거리와 FollowSpeed/DistanceStep은 GrabComponent가 관리한다. 새로운 일반 Pawn에서는 GrabComponent 설정을 직접 사용한다.

## 종료와 정리

- Release: Grab 참조와 대상의 보유 피드백 정리, 이전 물리 상태 복원.
- 대상 Destroy: OnDestroyed로 Grab 참조를 즉시 정리.
- Character UnPossessed/EndPlay: 선택/Grab 해제, 자신이 설치한 입력 바인딩과 Context 정리.
- 라운드 종료: 선택·보유 표시 정리, 택배 물리 정지, 이동·상호작용 차단.
- Restart: Grab/선택 해제 후 모든 택배 제거, 대차/통계/타이머/생성 초기화, 입력 상태 초기화.
- Feedback EndPlay/Destroy: 생성한 테두리 메시와 MID 정리.

## 수동 검증 체크리스트 — 미실행

| 확인 | 기대 결과 |
|---|---|
| 기존 맵 + 네이티브 Pawn | WASD/마우스/E/클릭/휠/R fallback |
| Interact만 F로 지정 | F만 상호작용, E/클릭 추가 실행 없음, WASD 유지 |
| Move/Look도 Enhanced 지정 | Enhanced 감도·반전·방향 적용, 기본 입력 중복 없음 |
| E에 다른 Enhanced Action 지정 | E의 기본 상호작용 차단 |
| 활성 IMC 제거/복구, 실제 키 remap | 완료된 매핑 재구축에 맞게 fallback/Enhanced 전환 |
| E와 클릭 동시 입력, Enhanced 버튼 길게 누르기 | Grab 직후 Release 등의 중복 없음 |
| 택배 조준 전환, Grab/Release | 하늘색 선택 / 금색 보유 / 다른 택배 강조 억제 |
| 대차 벽 / 버튼 | 벽은 출발 불가, 버튼만 출발 |
| 잡은 대상 파괴 / Pawn 빙의 해제 | 유효하지 않은 보유 참조·남은 강조 없음 |
| 종료 후 입력, Restart 클릭을 누른 상태 | 신규 상호작용 없음, 재시작 클릭으로 택배를 바로 잡지 않음 |
| 컨베이어/대차/점수 | 기존 규칙과 동일 |
| 작은 창/다른 해상도 | HUD 키 이름과 결과 버튼 가독성 |

사용자 요청에 따라 택배 샘플의 자동화 테스트 소스를 제거했다. 입력/카메라, 실제 Chaos 충돌, 조명·테두리 밝기, Blueprint 직렬화는 위 수동 검증 대상이다.
