# QML Hot Reload
实时预览和更新 QML 界面，高效开发和即时调试。

# 编译
## 自动化脚本
```cmd
:: cmd
.\script\01_build.bat && .\script\02_pack.bat && .\script\03_md5.bat && cd .\out\build\bin\
```
```powershell
# powershell
.\script\01_build.bat; .\script\02_pack.bat; .\script\03_md5.bat; cd .\out\build\bin\
```

# 常见问题
- 不同使用场景下该如何选择下载？
[下载链接](https://github.com/Mrliu88888888/QML-Hot-Reload/releases/tag/v1.1.0)
```txt
1. QHotReload-QT版本.exe
   如果你已经安装了Qt开发环境，只需下载与你Qt版本匹配的可执行文件，并将其拷贝到Qt的bin目录下即可使用。
2. QHotReload-QT版本-版本-编译配置-处理器架构.exe
   如果你没有安装Qt开发环境，建议下载此全量安装包。适用于QML学习、演示、教学等场景，安装后即可直接使用。
3. QHotReload.md5
   包含可执行文件和安装包的MD5校验信息。下载此文件可用于验证下载文件的完整性，确保文件未被篡改。
4. Source code.zip
   项目的源码文件，推荐在Windows系统下使用。
5. Source code.tar.gz
   项目的源码文件，推荐在Linux系统下使用。
```
- 在开发过程中，主屏幕用于编写代码，如何将 QML 界面的预览显示在第二块屏幕上？
``` txt
直接在预览QML源文件中设置x,y坐标

代码片段示例:
Window {
    x: Qt.application.screens[1].virtualX + 50
    y: Qt.application.screens[1].virtualY + 50
}

参考链接:
https://blog.csdn.net/u014037733/article/details/133082740
```

# 致谢
* [qhot](https://github.com/patrickelectric/qhot)
* [RuntimeCompiledCPlusPlus](https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus)
