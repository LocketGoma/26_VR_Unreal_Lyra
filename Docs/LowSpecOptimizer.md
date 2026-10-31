# 저사양 PC용 프로젝트·에디터 설정

`Scripts/optimize_low_spec.py`는 Windows/macOS/Linux에서 공용으로 쓰는 Python 3.9 이상 스크립트다. 외부 패키지가 필요 없으며, Unreal Editor의 내장 Python에서도 실행한다. 피쳐 폴더명이 달라도 현재 열린 프로젝트 또는 상위 `.uproject`를 탐색한다.

## 에디터에서 실행 — 권장

1. `Edit → Plugins`에서 **Python Editor Script Plugin**을 활성화하고 에디터를 재시작한다. 이미 활성화되어 있으면 생략한다.
2. `Tools → Execute Python Script`에서 이 피쳐의 **`Scripts/optimize_low_spec.py`**를 선택한다.
3. `Output Log`의 `[LowSpec]` 메시지에서 적용 대상·백업 위치를 확인한다.
4. 작업을 저장하고 에디터를 재시작한다. 레이트레이싱·거리 필드·셰이더 워커 설정은 재시작 후 적용된다.

현재 세션의 품질·프레임·후처리 설정은 즉시 적용한다. 에셋 창이나 머티리얼 창을 강제로 닫거나 에셋을 자동 저장하지 않는다. 불필요하게 열린 에셋 창은 직접 닫으면 RAM 사용량을 더 줄일 수 있다.

## 기본 적용값

| 항목 | 설정 |
|---|---|
| Engine Scalability | 모든 품질 그룹 Low(0) |
| 렌더 해상도 | 가로·세로 각각 50% — 전체 픽셀 수는 약 25% |
| 최대 프레임 | 30 FPS |
| 게임 창 기본 크기 | 1280 × 720, 창 모드 — 다음 실행에 적용 |
| Lumen GI·반사 / 하드웨어 RT | 끔 |
| 메시 거리 필드 생성 / 가상 그림자 / 일반 그림자 | 끔 |
| AA·모션 블러·DOF·블룸·AO·SSR·볼류메트릭 효과 | 끔 |
| 텍스처 스트리밍 예산 | 256 MiB |
| 렌더 타깃 풀 최소 보유량 | 100 MiB — 최대 메모리 제한이 아님 |
| 풀·수풀 밀도 | 0 |
| 셰이더 컴파일 워커 | 최대 4개 목표, 논리 코어 최소 1개는 남김 |
| 에셋 탭 자동 복원 / 소스 에셋 자동 재임포트 감시 | 끔 |
| 에디터 세션 설정 | 백그라운드 CPU 절약, 실시간 썸네일·머티리얼 프리뷰 끔, 메모리 표시 켬 |

셰이더 컴파일은 유지하며 DDC를 삭제하지 않는다. 최초 실행은 셰이더·에셋 준비 때문에 여전히 오래 걸릴 수 있다. UE 자체의 저코어 PC 예외 처리 등에 따라 실제 셰이더 워커 수는 요청값과 다를 수 있다. 이 값은 C++ 빌드 병렬도와 별개다.

**256 MiB는 전체 RAM/VRAM 제한이 아니다.** 스트리밍되지 않는 텍스처·메시·에디터 모듈·셰이더 등은 별도로 메모리를 쓴다. Unreal Editor는 일부 텍스처 MipBias/VRAM 제한을 게임 실행과 다르게 처리한다. 작업 관리자에서 무조건 특정 메모리 이하가 된다는 보장은 없다.

자동 저장은 유지한다. 엔진·Lyra 소스, 에셋, 플러그인 활성 목록은 수정하지 않는다. Nanite·Virtual Texture·Substrate 등 에셋 호환성에 민감한 기능 중 Nanite만 명시적 추가 옵션으로 끌 수 있다.

## Windows 터미널에서 실행

에디터를 닫고 피쳐 폴더에서 실행한다. 별도 Python 3.9 이상이 필요하다. Windows Python Launcher가 없으면 `py -3` 대신 `python`을 사용한다.

```powershell
# 미리보기: 파일 변경 없음
py -3 Scripts/optimize_low_spec.py preview

# 기본 설정 적용
py -3 Scripts/optimize_low_spec.py apply

# 필요에 따라 조정: 워커를 8개까지 허용
py -3 Scripts/optimize_low_spec.py apply --shader-workers 8

# 더 낮은 실행 부하: 20 FPS, 35% 렌더 해상도, 128 MiB 텍스처 예산
py -3 Scripts/optimize_low_spec.py apply --fps 20 --screen 35 --pool-mb 128

# 추가 절약: Nanite 대신 fallback 메시 사용. 일부 메시가 사라지거나 깨질 수 있음
py -3 Scripts/optimize_low_spec.py apply --disable-nanite

# 최초 적용 전 설정으로 복원 — 에디터를 닫은 상태에서 실행
py -3 Scripts/optimize_low_spec.py restore
```

macOS/Linux에서는 `py -3` 대신 `python3`를 사용한다. 다른 프로젝트를 지정하려면 `--project "D:/Projects/Lyra/LyraStarterGame.uproject"`를 추가한다. 에디터 내부에서는 현재 열린 프로젝트에만 적용한다.

에디터 Python 콘솔에서 옵션을 바꾸려면 경로를 실제 피쳐 위치로 바꿔 실행한다. `run_name`을 지정하므로 기본값이 먼저 자동 적용되지 않는다.

```python
import runpy, unreal
from pathlib import Path
script = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())) / "Plugins/GameFeatures/VRSimulateSample/Scripts/optimize_low_spec.py"
optimizer = runpy.run_path(str(script), run_name="low_spec_optimizer")
optimizer["apply"](shader_workers=8, fps=30, screen=50, pool_mb=256)
# 파일·현재 세션을 바꾸지 않고 미리보기만 하려면:
# optimizer["apply"](dry_run=True)
```

## 저장·백업 범위

- 프로젝트 `Config/DefaultEngine.ini`, `DefaultGameUserSettings.ini`, `DefaultEditorPerProjectUserSettings.ini`를 변경한다. 프로젝트 공통 설정이므로 커밋·패키징 시에도 영향을 준다.
- 현재 OS의 `Saved/Config/<플랫폼>` 및 `<플랫폼>Editor` 사용자 설정도 갱신한다. 이미 존재하는 해당 폴더의 `Engine.ini`도 갱신한다.
- 원본은 프로젝트 **`Saved/LowSpecOptimizer/original/`**, 목록은 **`manifest.json`**에 보관한다. 반복 적용해도 첫 백업을 유지한다. 백업 폴더는 `Saved`에 있으므로 복원 전 삭제하지 않는다.
- 복원은 변경 대상 **INI 파일 전체**를 첫 적용 전 상태로 되돌린다. 적용 후 다른 설정을 바꿨다면 그 값도 되돌아간다. 복원 직전 파일은 `Saved/LowSpecOptimizer/restored-<시각>/before-restore/`에 별도로 남긴다.
- 에디터 설정 캐시가 복원본을 덮어쓰는 것을 막기 위해 복원은 에디터를 종료한 뒤 터미널에서 실행한다. 외부 적용도 에디터를 종료한 상태에서 실행한다.
- 엔진 전역 `EditorSettings.ini`는 수정하지 않는다. 백그라운드 CPU·썸네일 등 Python으로 바꾸는 에디터 선호 설정은 현재 세션용이므로 다음 세션에서도 필요하면 스크립트를 다시 실행한다. 프로젝트 렌더링 제한은 파일에 저장된다.

품질·FPS 등의 CVar는 `SystemSettings`에 명시하므로 프로파일 사용 중 일반 품질 메뉴보다 우선할 수 있다. 제한을 풀 때는 복원하거나 이 스크립트로 다시 설정한다. 실사용 RAM을 더 줄여야 한다면 에디터 PIE보다 에디터를 닫고 패키징한 Development/Shipping 실행 파일만 실행하는 방식이 유리하다.

## 참고

- [Epic: Python으로 Unreal Editor 스크립팅](https://dev.epicgames.com/documentation/unreal-engine/scripting-the-unreal-editor-using-python)
- [Epic: Scalability Reference — Low=0](https://dev.epicgames.com/documentation/unreal-engine/scalability-reference-for-unreal-engine)
- 설정 이름은 로컬 UE 5.8.2 헤더·설정 파일을 참고했다. Python 문법 확인만 수행하며 에디터 실행·성능 측정은 수행하지 않았다.
