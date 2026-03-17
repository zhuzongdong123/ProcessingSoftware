; 脚本由 Inno Setup 脚本向导 生成！
; 有关创建 Inno Setup 脚本文件的详细资料请查阅帮助文档！

#define MyAppName "智能标绘"
#define MyAppVersion "1.1.0"
#define MyAppPublisher "山东高速"
#define MyAppURL "http://www.sdgs.com/"
#define MyAppExeName "ProcessingSoftware.exe"

[Setup]
; 注: AppId的值为单独标识该应用程序。
; 不要为其他安装程序使用相同的AppId值。
; (若要生成新的 GUID，可在菜单中点击 "工具|生成 GUID"。)
AppId={{6914B9FE-CD44-42AD-909A-5FF6D40C5E69}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName=D:\{#MyAppName}
DisableProgramGroupPage=yes
; 以下行取消注释，以在非管理安装模式下运行（仅为当前用户安装）。
PrivilegesRequired=admin
OutputDir=D:\03.安装包\0 蓝盾
OutputBaseFilename={#MyAppName}_{#MyAppVersion}_20251023
SetupIconFile="E:\work\02.code\ProcessingSoftware-new\client\tubiao.ico"
Compression=lzma
SolidCompression=yes
WizardStyle=modern

;写入注册表代码
[Registry]
;以管理员身份运行安装路径下的软件
Root: HKCU; Subkey: "SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers"; ValueType:string;ValueName:"{app}\{#MyAppExeName}";ValueData:"RUNASADMIN"
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "{#MyAppName}"; ValueData: "{app}\{#MyAppExeName}"; Tasks: startupicon; Flags: uninsdeletevalue

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "DesktopIcon"; Description: "创建桌面快捷方式"; GroupDescription: "附加任务";
Name: "startupicon"; Description: "开机启动"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
Source: "E:\work\02.code\ProcessingSoftware-new\build-ProcessingSoftware-Desktop_Qt_5_14_2_MSVC2017_64bit-Release\software\ProcessingSoftware.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "E:\work\02.code\ProcessingSoftware-new\build-ProcessingSoftware-Desktop_Qt_5_14_2_MSVC2017_64bit-Release\software\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; 注意: 不要在任何共享系统文件上使用“Flags: ignoreversion”

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{commonstartup}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: startupicon

[Run]
Filename: "{app}\update.bat"; Description: "更新文件"; StatusMsg: "更新文件"; Parameters: "{src}\update {app}";
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait  skipifsilent

[UninstallRun]
; 卸载前杀掉进程
Filename: taskkill;Parameters:"/t /f /im {#MyAppExeName}";Flags: runhidden
Filename: taskkill;Parameters:"/t /f /im QtWebEngineProcess.exe";Flags: runhidden

[UninstallDelete]
; 卸载后删除安装目录下所有文件
Type: filesandordirs; Name: "{app}"

