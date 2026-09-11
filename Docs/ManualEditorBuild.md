# Lyra Editor 수동 빌드 및 실행

VRSimulate 루트의 **BuildAndOpenLyraEditor.bat**을 더블클릭한다.
현재 프로젝트 에디터를 닫은 상태에서 실행한다. 스크립트 파일이 있는 폴더부터 상위로 올라가 가장 가까운 .uproject를 찾는다. 바로가기의 작업 폴더에 의존하지 않는다.

순서: 프로젝트 검색 → 엔진 선택 → **LyraEditor / Win64 / Development** 빌드 → 성공 시 UnrealEditor로 프로젝트 열기.
VRSimulate를 포함하도록 UBT에 **-EnablePlugin=VRSimulate**를 전달한다.
실패하면 에디터를 열지 않고 종료 코드를 표시한다. 창은 키를 누를 때까지 유지된다.
패키징·Cook·자동화 테스트·프로젝트 파일 생성은 실행하지 않는다.

## 엔진 선택 순서

1. 명시한 -EngineRoot 또는 환경 변수 UE_ENGINE_ROOT.
2. .uproject의 EngineAssociation: 등록된 소스 엔진 GUID, 런처 버전, 엔진 경로.
3. EngineAssociation이 비어 있으면 프로젝트를 포함한 상위 소스 엔진.
4. 연결 엔진을 찾지 못하면 **현재 PC에 설치된 런처 엔진 중 최신 버전**.
5. 사용 가능한 엔진이 없으면 콘솔에서 엔진 루트 폴더 입력. 빈 입력은 취소.

소스 엔진 등록은 현재 사용자의 Unreal Engine Builds 레지스트리에서 확인한다.
런처 설치는 LauncherInstalled.dat와 32/64비트 Unreal Engine 설치 레지스트리를 확인한다.
버전은 각 엔진의 Engine/Build/Build.version을 숫자로 비교한다. 오래된 설치 기록 중 실제 Build.bat이 없는 경로는 제외한다.

“최신”은 설치된 런처 엔진 기준이며 인터넷에서 새 엔진을 다운로드하지 않는다.
최신 런처 엔진이 프로젝트 소스와 호환된다는 의미는 아니다. fallback 시 경고를 표시하며 EngineAssociation이나 Target.cs를 자동 수정하지 않는다.

## 선택 옵션

Windows 명령 프롬프트에서 VRSimulate 폴더로 이동한 뒤:

~~~bat
rem Engine folder override for this run.
BuildAndOpenLyraEditor.bat -EngineRoot "F:\Programing\UE5.8"

rem Editor build without launching the editor.
BuildAndOpenLyraEditor.bat -NoLaunch

rem Detection only: no build or editor launch.
BuildAndOpenLyraEditor.bat -DetectOnly

rem Explicit engine detection only.
BuildAndOpenLyraEditor.bat -EngineRoot "F:\Programing\UE5.8" -DetectOnly
~~~

자동 탐색이 불가능하면 더블클릭 후 표시되는 Engine root folder 입력란에 엔진 루트를 붙여 넣는다.
예: F:\Programing\UE5.8. Engine 하위 폴더 자체가 아니라 **Engine 폴더를 포함한 부모**를 지정한다.

여러 .uproject가 같은 가장 가까운 폴더에 있으면 임의 선택하지 않고 중단한다.
Unreal의 Build.bat 제한에 따라 프로젝트/엔진 경로에 느낌표, 퍼센트, 큰따옴표는 사용할 수 없다.

## 파일과 환경

- BuildAndOpenLyraEditor.bat: 더블클릭 진입점, 결과 창 유지.
- Scripts/Build-LyraEditor.ps1: 프로젝트/엔진 검색과 빌드·에디터 실행.
- 두 파일의 상대 위치를 유지한다.
- Windows 기본 PowerShell 5.1을 사용한다. ExecutionPolicy Bypass는 해당 프로세스에만 적용한다.
- Visual Studio의 C++ 도구와 Windows SDK 등 실제 빌드 환경은 미리 설치되어 있어야 한다.
- 빌드 실행 시 일반 Unreal 빌드와 동일하게 Binaries/Intermediate 등이 생성된다.

## 확인 범위

PowerShell 문법과 -DetectOnly 경로만 확인했다. 실제 빌드·에디터 실행·더블클릭 실행은 하지 않았다.
현재 점검 환경에서는 프로젝트의 GUID 연결과 유효한 런처 설치를 자동으로 찾지 못했다.
명시한 F:\Programing\UE5.8 경로는 UE 5.8.1로 확인되었으며, 올바른 프로젝트와 빌드 명령을 출력했다.
