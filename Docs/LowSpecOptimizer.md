# 저사양 PC용 프로젝트·에디터 설정

`Scripts/optimize_low_spec.py`는 Windows/macOS/Linux에서 공용으로 쓰는 Python 3.9 이상 스크립트다. 외부 패키지가 필요 없으며, Unreal Editor의 내장 Python에서도 실행한다. 피쳐 폴더명이 달라도 현재 열린 프로젝트 또는 상위 `.uproject`를 탐색한다.

## 에디터에서 실행 — 권장

1. `Edit → Plugins`에서 **Python Editor Script Plugin**을 활성화하고 에디터를 재시작한다. 이미 활성화되어 있으면 생략한다.
2. `Tools → Execute Python Script`에서 이 피쳐의 **`Scripts/optimize_low_spec.py`**를 선택한다.
3. `Output Log`의 `[LowSpec]` 메시지에서 적용 대상·백업 위치를 확인한다.
4. 작업을 저장하고 에디터를 재시작한다. 모든 그래픽·프레임·셰이더 워커 설정은 재시작 후 적용된다.

현재 세션에서는 품질·프레임·후처리 CVar나 GameUserSettings를 변경하지 않는다. Niagara 등의 프리뷰가 실행 중인 상태에서 품질을 일괄 전환하는 경로를 피하기 위한 조치다. 에셋 창이나 머티리얼 창을 강제로 닫거나 에셋을 자동 저장하지 않는다. 불필요하게 열린 에셋 창은 직접 닫으면 RAM 사용량을 더 줄일 수 있다.

### Niagara 호환성 보완

기존 버전은 이미터 에셋을 직접 비활성화하지는 않았지만, `sg.EffectsQuality=0`으로 설정했다. UE 5.8.2의 Low Effects는 `fx.Niagara.QualityLevel=0`, `r.DetailMode=0`, 저품질 머티리얼 설정 등을 함께 적용한다. 에셋별 Scalability 설정에 따라 이미터가 실행 대상에서 제외될 수 있다. 이것만으로 NiagaraEditor 크래시의 원인이 확정되는 것은 아니다.

수정 버전은 **Effects 품질을 중간(Medium, `sg.EffectsQuality=1`)으로 고정**한다. 이전 적용값이나 백업이 Low·Epic이어도 중간으로 갱신한다. 거리 필드 기반 Niagara GPU 충돌을 고려해 `r.GenerateMeshDistanceFields`는 원본 설정을 복구·보존한다. 이 예외 때문에 전체 Low 버전보다 메모리나 이펙트 비용이 늘 수 있다. 중간 품질 자체가 Niagara 크래시 해결을 보장하는 것은 아니다.

다른 PC에서 이전 버전으로 적용했다면 **수정된 Python 파일을 그 PC에도 복사**하고, 에디터를 닫은 상태에서 배치파일이나 `apply` 명령을 다시 실행한다. 해당 PC의 `Saved/LowSpecOptimizer` 백업은 그대로 둔다. Effects는 중간으로 갱신하고 거리 필드는 기존 백업에서 원본 값을 읽어 복구한다. 스크립트 파일을 교체하는 것만으로는 기존 INI가 바뀌지 않는다. 크래시 복구 후 바로 에셋을 다시 열기보다 빈 맵으로 먼저 시작하는 기본 배치 실행을 권장한다.

## 기본 적용값

| 항목 | 설정 |
|---|---|
| Engine Scalability | Effects를 제외한 품질 그룹 Low(0) |
| Effects / Niagara 품질 | 중간(Medium, 1) |
| 렌더 해상도 | 가로·세로 각각 50% — 전체 픽셀 수는 약 25% |
| 최대 프레임 | 30 FPS |
| 게임 창 기본 크기 | 1280 × 720, 창 모드 — 다음 실행에 적용 |
| Lumen GI·반사 / 하드웨어 RT | 끔 |
| 메시 거리 필드 생성 | 원본 설정 보존 — Niagara GPU 충돌 호환성 |
| 가상 그림자 / 일반 그림자 | 끔 |
| AA·모션 블러·DOF·블룸·AO·SSR·볼류메트릭 효과 | 끔 |
| 텍스처 스트리밍 예산 | 256 MiB |
| 렌더 타깃 풀 최소 보유량 | 100 MiB — 최대 메모리 제한이 아님 |
| 풀·수풀 밀도 | 0 |
| 셰이더 컴파일 워커 | 최대 4개 목표, 논리 코어 최소 1개는 남김 |
| 에셋 탭 자동 복원 / 소스 에셋 자동 재임포트 감시 | 끔 |
| 에디터 시작 설정 | 배치 실행 시 백그라운드 CPU 절약·썸네일 끄기·메모리 표시. 머티리얼 프리뷰 끄기는 프로젝트 INI에 저장 |

셰이더 컴파일은 유지하며 DDC를 삭제하지 않는다. 최초 실행은 셰이더·에셋 준비 때문에 여전히 오래 걸릴 수 있다. UE 자체의 저코어 PC 예외 처리 등에 따라 실제 셰이더 워커 수는 요청값과 다를 수 있다. 이 값은 C++ 빌드 병렬도와 별개다.

**256 MiB는 전체 RAM/VRAM 제한이 아니다.** 스트리밍되지 않는 텍스처·메시·에디터 모듈·셰이더 등은 별도로 메모리를 쓴다. Unreal Editor는 일부 텍스처 MipBias/VRAM 제한을 게임 실행과 다르게 처리한다. 작업 관리자에서 무조건 특정 메모리 이하가 된다는 보장은 없다.

자동 저장은 유지한다. 엔진·Lyra 소스, 에셋, 플러그인 활성 목록은 수정하지 않는다. Nanite·Virtual Texture·Substrate 등 에셋 호환성에 민감한 기능 중 Nanite만 명시적 추가 옵션으로 끌 수 있다.

## Windows 터미널에서 실행

### 에디터 로드 단계까지 줄이는 배치파일

에디터를 닫고 피쳐 루트의 **`OpenLowSpecLyraEditor.bat`**을 더블클릭한다. 같은 폴더의 `Scripts/Start-LowSpecEditor.ps1`과 `Scripts/optimize_low_spec.py`가 함께 있어야 한다.

프로젝트의 `EngineAssociation`에 연결된 Windows 엔진을 탐색하고, **엔진에 포함된 Python으로 에디터 실행 전에** 저사양 프로파일을 적용·백업한다. 시스템 Python이나 에디터 Python 플러그인을 별도로 활성화할 필요가 없다. 엔진에 Python 실행 파일이 빠져 있으면 오류를 표시한다.

기본값은 `/Engine/Maps/Entry` 빈 맵으로 시작한다. 시작부터 큰 맵의 에셋을 불러오는 작업을 줄이기 위한 동작이며, 에디터가 열리면 작업할 맵을 직접 열면 된다. 작업 맵을 나중에 여는 경우에는 그 맵의 로딩 비용이 발생한다.

시작 인자로 에셋 탭 자동 복원, 소스 에셋 자동 재임포트 감시, Live Coding 자동 시작·모듈 프리로드, 소스 관리 연결을 끈다. 에디터 성능 도구·실시간 썸네일을 끄고 저해상도·백그라운드 CPU 절약 설정도 시작 시 전달한다. 데스크톱 작업용으로 HMD 초기화를 건너뛴다. 사운드는 기본 유지한다.

```bat
:: 기본: 빈 맵으로 시작
OpenLowSpecLyraEditor.bat

:: 택배 분류 맵을 바로 열기
OpenLowSpecLyraEditor.bat -SampleMap

:: 시작할 맵 직접 지정
OpenLowSpecLyraEditor.bat -Map /Game/Maps/MyMap

:: 엔진 자동 탐색이 안 되는 PC
OpenLowSpecLyraEditor.bat -EngineRoot "C:\Program Files\Epic Games\UE_5.8"

:: 최초 준비 속도를 위해 셰이더 워커 수 늘리기
OpenLowSpecLyraEditor.bat -ShaderWorkers 8

:: 추가 절약: 사운드도 끄기
OpenLowSpecLyraEditor.bat -Mute

:: 소스 관리 / Live Coding을 기존 설정대로 사용할 경우
OpenLowSpecLyraEditor.bat -KeepSourceControl -EnableLiveCoding

:: 경로와 실행 명령 확인만: 설정 적용·에디터 실행 없음
OpenLowSpecLyraEditor.bat -DetectOnly
```

`-FPS 30 -Screen 50 -PoolMB 256`으로 그래픽 옵션도 바꿀 수 있다. `-SampleMap`과 `-Map`을 함께 주면 `-SampleMap`이 우선한다. 연결된 엔진을 찾지 못하면 임의의 다른 버전으로 실행하지 않고 `-EngineRoot` 지정을 안내한다.

이미 같은 프로젝트가 열려 있으면 중단한다. 시작 전 C++ 빌드를 별도로 실행하지 않으므로 코드 변경 후에는 IDE나 기존 `BuildAndOpenLyraEditor.bat`으로 먼저 빌드해야 한다. Unreal 자체의 필수 모듈 검사·셰이더 컴파일은 유지한다. DDC·셰이더 캐시·에셋 레지스트리를 삭제하지 않는다. 최초 준비에 필요한 작업을 생략하는 기능은 아니며 시작 시간 단축 수치를 측정하지 않았다.

프로파일 복원은 아래 복원 전용 스크립트나 `restore` 명령을 사용한다. 복원 후 기본 실행으로 돌아가려면 `.uproject`를 직접 연다. 저사양 실행 배치파일은 실행할 때마다 저사양 프로파일을 다시 적용한다.

배치파일은 Windows PowerShell 5.1을 사용한다. 현재 macOS 작업 환경에서 Windows 에디터 실행 검증은 수행하지 않았다.

### 복원 전용 스크립트

**에디터를 닫은 뒤 피쳐 루트의 `RestoreLowSpecSettings.bat`을 더블클릭한다.** 연결된 엔진의 내장 Python으로 최초 최적화 적용 전 설정을 복원하며 에디터는 자동 실행하지 않는다. 해당 프로젝트가 실행 중이거나 최초 백업이 없으면 중단한다.

같은 위치의 `Scripts/Start-LowSpecEditor.ps1`, `Scripts/restore_low_spec.py`, `Scripts/optimize_low_spec.py`가 필요하다. 문제 PC에도 이 파일들을 함께 복사하고, 그 PC의 기존 `Saved/LowSpecOptimizer` 백업을 유지한다. 다른 PC의 백업으로 교체하지 않는다.

```bat
:: 최초 최적화 적용 전 상태로 복원
RestoreLowSpecSettings.bat

:: 엔진 자동 탐색이 안 되는 PC
RestoreLowSpecSettings.bat -EngineRoot "C:\Program Files\Epic Games\UE_5.8"

:: 경로·백업 존재 여부만 확인, 실제 복원 없음
RestoreLowSpecSettings.bat -DetectOnly

:: 별도 Python으로 직접 복원할 경우
py -3 Scripts\restore_low_spec.py
```

macOS/Linux는 `python3 Scripts/restore_low_spec.py`로 실행한다. Python을 직접 호출할 때도 에디터는 미리 종료한다. 에디터 내부 실행은 설정 캐시가 복원본을 덮어쓰는 문제 때문에 거부한다.

복원은 대상 **INI 파일 전체**를 최초 백업으로 되돌리므로 이후 직접 바꾼 설정도 복원 대상에 포함된다. 현재 파일은 먼저 `Saved/LowSpecOptimizer/restored-<시각>/before-restore/`에 보관한다. 복원 완료 후에는 `.uproject`를 직접 열어 실행한다.

### Python을 직접 호출

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
- 엔진 전역 `EditorSettings.ini`는 수정하지 않는다. 백그라운드 CPU·썸네일 등의 에디터 선호 설정은 Windows 배치 실행 시 시작 인자로 전달한다. Python 파일 자체는 현재 에디터 세션의 선호 설정을 바꾸지 않는다. 프로젝트 렌더링 제한은 파일에 저장된다.

품질·FPS 등의 CVar는 `SystemSettings`에 명시하므로 프로파일 사용 중 일반 품질 메뉴보다 우선할 수 있다. 제한을 풀 때는 복원하거나 이 스크립트로 다시 설정한다. 실사용 RAM을 더 줄여야 한다면 에디터 PIE보다 에디터를 닫고 패키징한 Development/Shipping 실행 파일만 실행하는 방식이 유리하다.

## 참고

- [Epic: Python으로 Unreal Editor 스크립팅](https://dev.epicgames.com/documentation/unreal-engine/scripting-the-unreal-editor-using-python)
- [Epic: Scalability Reference — Low=0](https://dev.epicgames.com/documentation/unreal-engine/scalability-reference-for-unreal-engine)
- [Epic: 실행 인자와 시작 시 INI 설정 덮어쓰기](https://dev.epicgames.com/documentation/unreal-engine/configuration-files-in-unreal-engine)
- [Epic: Niagara 이미터의 품질별 Scalability 설정](https://dev.epicgames.com/documentation/unreal-engine/emitter-settings-reference-for-niagara-effects-in-unreal-engine)
- 설정 이름은 로컬 UE 5.8.2 헤더·설정 파일을 참고했다. Python 문법 확인만 수행하며 에디터 실행·성능 측정은 수행하지 않았다.
