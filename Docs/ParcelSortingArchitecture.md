# VRSimulate 컴포넌트 구조 — UE 5.8

택배 분류 샘플의 클래스 책임, 입력 연결, BP 확장 지점을 정리한다. 전체 설정 분류는 `README.md`, 게임 조작과 판정 규칙은 `Docs/ParcelSorting.md`를 참고한다.

파일 경로는 Feature 루트 기준이며, 기본 수치는 C++ 코드 기준이다.

## 1. 코드 구조와 수정 위치

### 클래스 책임

공통 컴포넌트는 `Base / Interaction / Grab / Feedback`, 샘플 규칙은 `ParcelSorting`에 있다. 헤더는 `Source/VRSimulateRuntime/Public`, 구현은 `Source/VRSimulateRuntime/Private`에서 찾는다.

| 클래스 | 책임 |
|---|---|
| `ParcelSortingGameMode` | Pawn/HUD 및 활성 Arena 선택 |
| `ParcelSortingArena` | 작업장·택배·대차 생성, 컨베이어, 라운드·통계 |
| `ParcelSortingCharacter` | 컴포넌트 구성, 입력 전달, 카메라 연결 |
| `ParcelCameraMode` | Lyra 기반 1인칭 카메라 모드 |
| `UParcelInteractionAbility` | GAS를 통한 Grab/Release/출발 진입점 |
| `VRSimControllerProxyComponent` | 탐색과 Grab에 사용할 시점 기준점 |
| `VRSimInputRouterComponent` | Enhanced Input 우선 적용, 기본 키 입력 |
| `VRSimRayInteractionComponent` | 조준 방향의 대상 탐색 |
| `VRSimInteractionComponent` | 선택 대상과 Hit 결과, 상호작용 전달 |
| `VRSimGrabComponent` | 보유 대상·거리·이동·물리 상태 |
| `IVRSimInteractable` | 선택·상호작용 인터페이스 |
| `IVRSimGrabbable` | 잡기·놓기 인터페이스 |
| `VRSimFeedbackComponent` | 피드백 확장 지점 |
| `VRSimOutlineFeedbackComponent` | 선택·보유 테두리 표시 |
| `ParcelPackage` | 분류·문자·처리 상태, 대상 인터페이스 구현 |
| `ParcelCart` | 적재 판정, 출발, 문·퇴장·복귀 |

### 실행 흐름

```text
Character.Tick
  → 시점을 ControllerProxy에 반영
  → RayInteraction.UpdateDetection
  → Interaction의 선택 상태 갱신
  → Grab.UpdateManipulation
```

HUD는 Interaction의 선택 정보를 사용한다. 선택한 택배는 하늘색, 보유한 택배는 금색 테두리로 표시한다. 대차는 출발 버튼을 조준한 경우에 상호작용한다.

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

Character가 탐색과 보유 물체 이동의 갱신 순서를 관리한다. GrabComponent는 보유 중 물체를 이동시키고 Release에서 물리 상태를 복원한다. 공통 Interaction/Grab 컴포넌트는 택배 분류·점수 규칙을 참조하지 않는다.

### C++ 변경 위치

| 목적 | 수정 대상 |
|---|---|
| 새 입력 명령 추가 | `EVRSimInputCommand`, InputRouter, Character의 `HandleInputCommand` |
| GAS 동작 변경 | `UParcelInteractionAbility`, Character의 `PerformInteraction` |
| 기본 탐색 방식 변경 | `VRSimRayInteractionComponent` |
| 기본 Grab 이동 방식 변경 | `VRSimGrabComponent` |
| 기본 테두리 표현 변경 | `VRSimOutlineFeedbackComponent` |
| 생성할 택배·대차 클래스 변경 | Arena의 `SpawnPackage`, `BuildWorkstation` |
| 게임 판정 변경 | `ParcelCart`, `ParcelSortingArena`, `ParcelSortingTypes` |

## 2. BP 기본값에서 수정하는 항목

### Character 설정

| 위치 | 속성 | 용도 |
|---|---|---|
| Character BP | `InteractionDistance` | 탐색 거리. 기본 350cm (코드) |
| Character BP | `HoldDistance` | 보유 거리. 기본 200cm (코드) |
| Grab | `MinHoldDistance` | 최소 보유 거리 |
| Grab | `DistanceStep`, `FollowSpeed` | 거리 조절량·추적 속도 |
| Interaction | `TraceChannel`, `Debug Interaction` | 탐색 채널·디버그 표시 |
| CameraComponent | `Field Of View` | 목표 FOV. Offset 자동 계산 |

탐색 거리와 기본 보유 거리는 Character 속성에서 지정하며 Play 시작 시 적용한다.

### Enhanced Input 연결

1. 사용할 `ParcelSortingCharacter` 파생 BP를 선택한다.
2. 아래 타입에 맞는 Input Action과 Input Mapping Context를 준비한다.
3. Character BP → InputRouter → Actions에서 명령별 Action을 지정한다.
4. 같은 컴포넌트의 MappingContext에 IMC를 지정한다. MappingPriority 기본값은 0이다.
5. GameMode BP의 Default Pawn Class에 해당 Character BP를 지정한다.
6. 맵의 GameMode Override에 해당 GameMode BP를 지정한다.

| 명령 | Action 이름 예시 | 값 타입 | 값 계약 |
|---|---|---|---|
| Move | `IA_Parcel_Move` | Axis2D | X=오른쪽, Y=앞쪽 |
| Look | `IA_Parcel_Look` | Axis2D | X=Yaw, Y=Pitch |
| Interact | `IA_Parcel_Interact` | Boolean | Grab/Release/대차 버튼 |
| AdjustDistance | `IA_Parcel_Reach` | Axis1D | 양수=멀리, 음수=가까이 |
| Reset | `IA_Parcel_Reset` | Boolean | 종료 화면에서 재시작 |

IMC 매핑 예시:

| Action | Key | Modifier |
|---|---|---|
| Move | D | 없음 |
| Move | A | Negate X |
| Move | W | Swizzle Input Axis Values: YXZ |
| Move | S | Negate X, 다음 Swizzle YXZ |
| Look | Mouse XY 2D-Axis | Negate Y |
| Interact | E 또는 지정 키 | 없음 |
| AdjustDistance | Mouse Wheel Axis | 없음 |
| Reset | R 또는 지정 키 | 없음 |

마우스 감도·반전은 IMC Modifier에서 설정한다. Boolean 동작은 눌린 동안 Triggered가 발생하는 설정을 사용한다. Interact/Reset은 누름당 한 번 전달한다.

활성 Enhanced Input 매핑이 있는 명령은 해당 매핑을 사용한다. 매핑이 없는 명령은 기본 키를 사용하며, `bEnableBuiltInFallback=false`이면 기본 키 입력을 끈다. 예를 들어 Interact만 F로 지정하면 상호작용은 F를 사용하고 이동은 기본 WASD를 사용한다.

## 3. BP 커스텀으로 수정하는 항목

### 상호작용 대상

새 대상은 다음 인터페이스로 연결한다.

| 인터페이스 | 구현 지점 | 책임 |
|---|---|---|
| `VRSimInteractable` | `CanInteract` | 사용 가능 여부 |
| 동일 | `OnSelected`, `OnDeselected` | 선택 표시·해제 |
| 동일 | `Interact` | 상호작용 실행 |
| `VRSimGrabbable` | `CanGrab` | 잡기 허용 여부 |
| 동일 | `GetGrabPrimitive` | 이동할 Movable 루트 Primitive 반환 |
| 동일 | `OnGrabbed`, `OnReleased` | 보유 상태·표시 갱신 |

일반 잡기 대상과 택배 판정 대상은 구분한다. 컨베이어·분류·출발 집계는 `ParcelPackage`를 대상으로 한다.

`VRSimInteractableTarget`의 `On Selection Changed`, `On Interaction` 이벤트로 선택·상호작용 동작을 구성할 수도 있다.

### 컴포넌트 확장

| 컴포넌트 | BP 확장 지점 |
|---|---|
| Interaction | `UpdateDetection` |
| Grab | `TryGrab`, `TryRelease`, `UpdateManipulation` |
| Feedback | `PlaySelectionFeedback`, `PlayInteractionFeedback`, `PlayDestinationFeedback` |

새 Pawn에서 공통 기능을 사용할 때는 Proxy → Interaction → Grab을 연결한다. 샘플 Character의 기본 컴포넌트를 다른 클래스로 교체하는 작업은 C++ 구성에서 처리한다.

### 라운드·UI 연결

Arena의 `Get Remaining Time`, `Stats`, `OnRoundEnded`, `OnRoundRestarted`로 UI를 연결한다. Restart 버튼은 `Restart Round`를 호출한다. FOV의 런타임 변경은 CameraComponent의 `Set Field Of View`를 사용한다.

## 4. 그 외: 상태 정리·참고 문서

| 시점 | 처리 |
|---|---|
| Release | 보유 상태·표시 해제, 물리 복원 |
| 대상 제거 | Grab 참조 정리 |
| 빙의 해제·Character 종료 | 선택·Grab 해제 |
| 라운드 종료 | 선택·보유 표시 정리, 이동·상호작용·택배 물리 정지 |
| Restart | 택배 제거, 대차·통계·타이머·생성·입력 초기화 |

현재 판정 규칙과 Arena 설정은 `Docs/ParcelSorting.md`를 참고한다. 기본 게임 시간은 **300초 (코드)**이며 실제 제한 시간은 Arena의 Settings를 따른다.

### UE 5.8 공식 문서

| 참고 항목 | 공식 문서 |
|---|---|
| `BlueprintCallable`, `BlueprintNativeEvent` 등 | [Exposing Gameplay Elements to Blueprints](https://dev.epicgames.com/documentation/en-us/unreal-engine/exposing-gameplay-elements-to-blueprints-visual-scripting-in-unreal-engine?application_version=5.8) |
| C++·BP 인터페이스 | [Interfaces in Unreal Engine](https://dev.epicgames.com/documentation/unreal-engine/interfaces-in-unreal-engine?application_version=5.8) |
| Input Action, Mapping Context, Modifier, Trigger | [Enhanced Input](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine?application_version=5.8) |
