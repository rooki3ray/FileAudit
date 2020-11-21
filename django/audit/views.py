import json
import sqlite3

from django.shortcuts import render
from django.http import FileResponse, HttpResponse, JsonResponse

# Create your views here.

def read_db(request, table):
    page = int(request.GET["page"])
    limit = int(request.GET["limit"])
    sort = "ASC" if request.GET["sort"] == "+id" else "DESC"
    filepath = request.GET.get("filepath", "")

    print(page, limit, sort, filepath)

    conn = sqlite3.connect('test.db')
    conn.text_factory = bytes
    cur = conn.cursor()
    if not filepath: 
        sql = """select * from {} order by id {} limit {}, {}""".format(table, sort, limit * (page - 1), limit)
    else:
        sql = """select * from {} where filepath like '%{}%' order by id {} limit {}, {}""".format(table, filepath, sort, limit * (page - 1), limit)
    res_list = cur.execute(sql)
    return res_list, cur, conn, filepath

def exit_db(items, cur, conn, table, filepath=""):
    if not filepath:
        sql = """select count(id) from {}""".format(table)
    else:
        sql = """select count(id) from {} where filepath like '%{}%'""".format(table, filepath)
    total = cur.execute(sql).fetchall()[0][0]
    cur.close()
    conn.close()
    content = {
        "total": total,
        "items": items
    }
    response = HttpResponse(json.dumps(content), content_type="application/json")
    response['Access-Control-Allow-Origin'] = '*'  # 允许所有的域名地址
    response["Access-Control-Allow-Methods"] = "GET,POST,OPTIONS,PATCH,PUT"  # 允许的请求方式
    return response

def table_open(request):
    page = int(request.GET["page"])
    limit = int(request.GET["limit"])
    sort = "ASC" if request.GET["sort"] == "+id" else "DESC"

    print(page, limit, sort)

    conn = sqlite3.connect('test.db')
    conn.text_factory = bytes
    cur = conn.cursor()
    sql = "select * from open order by id {} limit {}, {}".format(sort, limit * (page - 1), limit)
    res_list = cur.execute(sql)
    items = []
    # id username uid commandname pid logtime filepath opentype openresult 
    for res in res_list:
        d = dict()
        d['id'] = res[0]
        d['logtime'] = res[5].decode()
        d['username'] = res[1].decode()
        d['uid'] = res[2]
        try:
            d['filepath'] = res[6].decode()
        except:
            d['filepath'] = 'encode error'
        d["content_short"] = "123"
        d["content"] = "no"
        d['command'] = res[3].decode()
        d["result"] = res[8].decode()
        d['pid'] = res[4]
        items.append(d)

    sql = "select count(id) from open"
    total = cur.execute(sql).fetchall()[0][0]
    cur.close()
    conn.close()
    content = {
        "total": total,
        "items": items
    }
    response = HttpResponse(json.dumps(content), content_type="application/json")
    response['Access-Control-Allow-Origin'] = '*'  # 允许所有的域名地址
    response["Access-Control-Allow-Methods"] = "GET,POST,OPTIONS,PATCH,PUT"  # 允许的请求方式
    # response["Access-Control-Allow-Headers"] = "Content-Type"  # 允许的headers
    return response

def delete(table, delete_id):
    conn = sqlite3.connect('test.db')
    cur = conn.cursor()
    sql = "delete from {} where id={}".format(table, delete_id)
    cur.execute(sql)
    cur.close()
    conn.commit()
    conn.close()
    response = HttpResponse(json.dumps({}), content_type="application/json")
    response['Access-Control-Allow-Origin'] = '*'  # 允许所有的域名地址
    response["Access-Control-Allow-Methods"] = "GET,POST,OPTIONS,PATCH,PUT"  # 允许的请求方式
    return response

def table_open_delete(request):
    delete_id = int(request.GET["id"])
    print(delete_id)
    response = delete("open", delete_id)
    return response

def table_close(request):
    res_list, cur, conn, filepath = read_db(request, "close")
    items = []
    # id username uid commandname pid logtime filepath opentype openresult 
    for res in res_list:
        d = dict()
        d['id'] = res[0]
        d['logtime'] = res[5].decode()
        d['username'] = res[1].decode()
        d['uid'] = res[2]
        try:
            d['filepath'] = res[6].decode()
        except:
            d['filepath'] = 'encode error'
        d["content_short"] = "123"
        d["content"] = "no"
        d['command'] = res[3].decode()
        d["result"] = res[8].decode()
        d['pid'] = res[4]
        items.append(d)
    response = exit_db(items, cur, conn, "close", filepath)
    return response


def table_close_delete(request):
    delete_id = int(request.GET["id"])
    print(delete_id)
    response = delete("close", delete_id)
    return response

def table_kill(request):
    res_list, cur, conn, filepath = read_db(request, "kill")
    items = []
    # id username uid commandname pid logtime filepath opentype openresult 
    for res in res_list:
        d = dict()
        d['id'] = res[0]
        d['logtime'] = res[8].decode()
        d['username'] = res[1].decode()
        d['uid'] = res[2]
        try:
            d['filepath'] = res[9].decode()
        except:
            d['filepath'] = 'encode error'
        d["content_short"] = "123"
        d["content"] = "no"
        d['command'] = res[3].decode()
        d["result"] = res[10].decode()
        d['pid'] = res[4]
        d['sig'] = res[6]
        d['pid_killed'] = res[7]
        items.append(d)
    response = exit_db(items, cur, conn, "kill", filepath)
    return response


def table_kill_delete(request):
    delete_id = int(request.GET["id"])
    print(delete_id)
    response = delete("kill", delete_id)
    return response

def table_mkdir(request):
    res_list, cur, conn, filepath = read_db(request, "mkdir")
    items = []
    # id username uid commandname pid logtime filepath opentype openresult
    for res in res_list:
        d = dict()
        d['id'] = res[0]
        d['logtime'] = res[5].decode()
        d['username'] = res[1].decode()
        d['uid'] = res[2]
        try:
            d['filepath'] = res[9].decode()
        except:
            d['filepath'] = 'encode error'
        d["content_short"] = "123"
        d["content"] = "no"
        d['command'] = res[3].decode()
        d["result"] = res[8].decode()
        d['pid'] = res[4]
        d['mode'] = res[6]
        d['dirpath'] = res[7].decode()
        items.append(d)
    response = exit_db(items, cur, conn, "mkdir", filepath)
    return response


def table_mkdir_delete(request):
    delete_id = int(request.GET["id"])
    print(delete_id)
    response = delete("mkdir", delete_id)
    return response
