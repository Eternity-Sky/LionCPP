#!/bin/bash
# 一键修复 Qt 平台插件环境脚本
# 用于修复 qt.qpa.plugin: Could not find the Qt platform plugin "dxcb" 等问题

QT_PLUGIN_PATH="/home/tengxun/Qt/6.9.1/gcc_64/plugins/platforms"

# 检查插件路径是否存在
if [ ! -f "$QT_PLUGIN_PATH/libqxcb.so" ]; then
    echo "[错误] 未找到 $QT_PLUGIN_PATH/libqxcb.so，请检查 Qt 安装路径！"
    exit 1
fi

# 检查依赖库
MISSING_LIBS=""
for LIB in libxcb-xinerama.so.0 libxkbcommon-x11.so.0; do
    if ! ldconfig -p | grep -q $LIB; then
        MISSING_LIBS="$MISSING_LIBS $LIB"
    fi
done
if [ -n "$MISSING_LIBS" ]; then
    echo "[提示] 缺少以下依赖库：$MISSING_LIBS"
    echo "请执行: sudo apt-get install libxcb-xinerama0 libxkbcommon-x11-0"
fi

# 设置环境变量并写入 ~/.bashrc（如未存在）
if ! grep -q QT_QPA_PLATFORM_PLUGIN_PATH ~/.bashrc; then
    echo "export QT_QPA_PLATFORM_PLUGIN_PATH=$QT_PLUGIN_PATH" >> ~/.bashrc
    echo "[已写入] export QT_QPA_PLATFORM_PLUGIN_PATH=$QT_PLUGIN_PATH 到 ~/.bashrc"
else
    echo "[已存在] ~/.bashrc 已包含 QT_QPA_PLATFORM_PLUGIN_PATH 相关设置"
fi

# 立即生效
export QT_QPA_PLATFORM_PLUGIN_PATH=$QT_PLUGIN_PATH

echo "[完成] QT_QPA_PLATFORM_PLUGIN_PATH 已设置为: $QT_PLUGIN_PATH"
echo "请重新打开终端或执行 source ~/.bashrc 后再运行 LionCPP！"
echo "如仍有问题，可手动运行:"
echo "  export QT_DEBUG_PLUGINS=1"
echo "  ./LionCPP"
