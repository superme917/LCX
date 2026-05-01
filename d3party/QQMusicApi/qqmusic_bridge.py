#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
QQMusic API Bridge for C++ Qt Application

通信协议:
    请求 (stdin):  {"cmd": "command_name", "params": {...}}
    响应 (stdout): {"code": 0, "data": {...}}
                  或
                  {"code": -1, "error": "错误信息"}

支持的命令:
    - ping              : 健康检查
    - songlist_detail   : 获取歌单详情
    - lyric             : 获取歌词
"""

from qqmusic_api import Client
from pydantic import BaseModel
import asyncio
import json
import sys

# 配置 UTF-8 输出
if sys.stdout.encoding != 'utf-8':
    sys.stdout.reconfigure(encoding='utf-8')

def to_dict(obj):
    """递归将 Pydantic 模型转换为可 JSON 序列化的字典"""
    if isinstance(obj, BaseModel):
        return obj.model_dump()
    elif isinstance(obj, dict):
        return {k: to_dict(v) for k, v in obj.items()}
    elif isinstance(obj, list):
        return [to_dict(item) for item in obj]
    elif hasattr(obj, '__dict__'):
        return {k: to_dict(v) for k, v in obj.__dict__.items()}
    else:
        return obj

async def main():
    """主循环：从 stdin 读取命令，输出结果到 stdout"""
    async with Client() as client:
        while True:
            line = sys.stdin.readline()
            if not line:
                break
                
            line = line.strip()
            if not line:
                continue

            # 解析 JSON 请求
            try:
                request = json.loads(line)
            except json.JSONDecodeError as e:
                response = {"code": -1, "error": f"JSON 解析错误: {e}"}
                print(json.dumps(response, ensure_ascii=False))
                sys.stdout.flush()
                continue
                
            cmd = request.get("cmd", "")
            params = request.get("params", {})

            if cmd == "songlist_detail":
                songlist_id = int(params.get("id"))
                result = await client.songlist.get_detail(songlist_id=songlist_id, onlysong=True, num=1000)
                response = {"code": 0, "data": to_dict(result)}
            elif cmd == "lyric":
                song_id = int(params.get("id"))
                result = await client.lyric.get_lyric(value=song_id, trans=True)
                response = {"code": 0, "data": to_dict(result.decrypt())}
            else:
                response = {"code": -1, "error": f"Unknown command: {cmd}"}
            
            # 输出响应
            print(json.dumps(response, ensure_ascii=False), flush=True)

if __name__ == "__main__":
    asyncio.run(main())