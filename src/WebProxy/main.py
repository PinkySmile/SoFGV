from aiohttp import web
import asyncio
import os
import time

HOST = os.getenv('HOST', '0.0.0.0')
PORT = int(os.getenv('PORT', 8080))
websockets = {}


class EchoServerProtocol(asyncio.BaseProtocol):
	def __init__(self, ws):
		self.ws = ws

	def connection_made(self, transport: asyncio.DatagramTransport):
		self.transport = transport

	def datagram_received(self, data, addr):
		if addr not in self.ws['udp_clients']:
			self.ws['udp_clients'][addr] = time.time()
		keys = list(self.ws['udp_clients'].keys())
		for addr in keys:
			if time.time() - self.ws['udp_clients'][addr] > 10:
				del self.ws['udp_clients'][addr]
		ip = bytes(int(f) for f in addr[0].split('.'))
		port = addr[1].to_bytes(2, 'big')
		asyncio.create_task(self.ws['root'].send_bytes(ip + port + data))

	def send(self, data, addr):
		self.transport.sendto(data, addr)


async def html(request):
	return web.FileResponse('SoFGV.html')


async def wasm(request):
	return web.FileResponse('SoFGV.wasm')


async def js(request):
	return web.FileResponse('SoFGV.js')


async def create_room(request):
	ws = web.WebSocketResponse(protocols=['binary'])
	await ws.prepare(request)
	loop = asyncio.get_running_loop()
	ws_obj = {
		'root': ws,
		'last_id': 0,
		'ws_clients': {},
		'udp_clients': {}
	}
	transport, protocol = await loop.create_datagram_endpoint(lambda: EchoServerProtocol(ws_obj), local_addr=('0.0.0.0', 0))
	ws_obj['transport'] = transport
	ws_obj['protocol'] = protocol
	host, hport = transport.get_extra_info('sockname')
	websockets[hport] = ws_obj

	print("New connection setup on port", hport)
	await ws.send_bytes(hport.to_bytes(2, 'big'))
	async for msg in ws:
		if msg.type == web.WSMsgType.BINARY:
			ip = f'{msg.data[0]}.{msg.data[1]}.{msg.data[2]}.{msg.data[3]}'
			port = int.from_bytes(msg.data[4:6], 'big')
			data = msg.data[6:]
			if ip == '0.0.0.0':
				if port in ws_obj['ws_clients']:
					await ws_obj['ws_clients'][port].send_bytes(b'\x00\x00\x00\x00' + hport.to_bytes(2, 'big') + data)
			else:
				if (ip, port) in ws_obj['udp_clients']:
					protocol.send(data, (ip, port))
		elif msg.type == web.WSMsgType.CLOSE:
			break
	print("Destroying connection on port", hport)
	transport.close()
	del websockets[hport]
	for w in list(ws_obj['ws_clients'].values()):
		await w.close()
	return ws


async def join_room(request):
	code = int(request.match_info['code'])
	if not code in websockets:
		return web.HTTPNotFound()
	ws = web.WebSocketResponse(protocols=['binary'])
	await ws.prepare(request)
	if not code in websockets:
		await ws
		return ws
	ws_obj = websockets[code]
	ws_obj['last_id'] += 1
	i = ws_obj['last_id']
	ws_obj['ws_clients'][i] = ws
	async for msg in ws:
		if msg.type == web.WSMsgType.BINARY:
			await ws_obj['root'].send_bytes(b'\0\0\0\0' + i.to_bytes(2, 'big') + msg.data[6:])
		elif msg.type == web.WSMsgType.CLOSE:
			break
	del ws_obj['ws_clients'][i]
	return ws


def main():
	app = web.Application()
	app.router.add_route('GET', '/', html)
	app.router.add_route('GET', '/SoFGV.js', js)
	app.router.add_route('GET', '/SoFGV.html', html)
	app.router.add_route('GET', '/SoFGV.wasm', wasm)
	app.router.add_route('GET', '/host', create_room)
	app.router.add_route('GET', r'/join/{code:\d+}', join_room)
	web.run_app(app, host=HOST, port=PORT)


if __name__ == '__main__':
	main()