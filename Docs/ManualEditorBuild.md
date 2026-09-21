# Lyra Editor 수동 빌드 및 실행 — Windows

VRSimulate를 포함한 Lyra 프로젝트의 에디터 빌드·실행 스크립트 사용법이다. 파일 경로는 Feature 루트 기준이다.

## 실행 방법

1. Visual Studio C++ 도구와 Windows SDK를 설치한다.
2. 현재 프로젝트 에디터를 닫는다.
3. Feature 루트의 `BuildAndOpenLyraEditor.bat`을 더블클릭한다.

스크립트가 있는 폴더부터 상위 폴더로 이동하며 가장 가까운 `.uproject`를 찾는다.

```text
프로젝트 검색 → 엔진 선택 → LyraEditor / Win64 / Development 빌드
  → 성공 시 UnrealEditor로 프로젝트 열기
```

빌드 실패 시 종료 코드를 표시하고 창을 유지한다. 이 스크립트는 에디터 빌드·실행 전용이다.

## 엔진 선택

| 우선순위 | 선택 기준 |
|---|---|
| 1 | `-EngineRoot` 옵션 또는 `UE_ENGINE_ROOT` 환경 변수 |
| 2 | 프로젝트의 `EngineAssociation` |
| 3 | 연결 정보가 비어 있으면 프로젝트를 포함하는 상위 소스 엔진 |
| 4 | 연결 엔진을 찾지 못하면 설치된 최신 런처 엔진 |
| 5 | 사용할 엔진이 없으면 콘솔에서 엔진 경로 입력 |

엔진 경로는 `Engine` 폴더를 포함한 부모 폴더를 지정한다. 런처의 최신 버전은 현재 PC에 설치된 버전 중에서 선택한다.

## 실행 옵션

| 옵션 | 동작 |
|---|---|
| 없음 | 프로젝트·엔진 탐색, 빌드, 성공 시 에디터 실행 |
| `-NoLaunch` | 빌드만 수행 |
| `-DetectOnly` | 프로젝트·엔진과 실행 예정 명령만 표시 |
| `-EngineRoot` | 사용할 엔진 루트 지정 |

Feature 폴더의 명령 프롬프트에서 실행한다.

```bat
BuildAndOpenLyraEditor.bat -NoLaunch
BuildAndOpenLyraEditor.bat -DetectOnly
```

`-EngineRoot` 뒤에는 사용할 엔진 루트를 전달한다. 자동 탐색이 어려운 경우 더블클릭 후 표시되는 Engine root folder 입력란으로 지정할 수도 있다.

## 파일 구성

| 파일 | 역할 |
|---|---|
| `BuildAndOpenLyraEditor.bat` | 더블클릭 실행, 결과 창 유지 |
| `Scripts/Build-LyraEditor.ps1` | 프로젝트·엔진 탐색, 빌드·에디터 실행 |

두 파일의 상대 위치를 유지한다. VRSimulate 포함 옵션은 스크립트에서 전달한다.

## 관련 문서

| 문서 | 내용 |
|---|---|
| `README.md` | 프로젝트 구조와 설정 분류 |
| `Docs/ParcelSorting.md` | 맵 생성과 게임 사용법 |
