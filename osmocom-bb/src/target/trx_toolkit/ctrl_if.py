# -*- coding: utf-8 -*-

# TRX Toolkit
# CTRL interface implementation
#
# (C) 2016-2020 by Vadim Yanitskiy <axilirator@gmail.com>
# Contributions by sysmocom - s.f.m.c. GmbH
#
# All Rights Reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

import logging as log

from udp_link import UDPLink

class CTRLInterface(UDPLink):
	def __init__(self, *udp_link_args):
		UDPLink.__init__(self, *udp_link_args)
		log.debug("Init TRXC interface (%s)" % self.desc_link())

	def handle_rx(self):
		# Read data from socket
		data, remote = self.sock.recvfrom(128)
		data = data.decode()

		if not self.verify_req(data):
			log.error("Wrong data on TRXC interface")
			return

		# Attempt to parse a command
		request = self.prepare_req(data)
		rc = self.parse_cmd(request)

		if type(rc) is tuple:
			self.send_response(request, remote, rc[0], rc[1])
		else:
			self.send_response(request, remote, rc)

	def verify_req(self, data):
		# Verify command signature
		return data.startswith("CMD")

	def prepare_req(self, data):
		# Strip signature, paddings and \0
		request = data[4:].strip().strip("\0")
		# Split into a command and arguments
		request = request.split(" ")
		# Now we have something like ["TXTUNE", "941600"]
		return request

	# If va is True, the command can have variable number of arguments
	def verify_cmd(self, request, cmd, argc, va = False):
		# Check if requested command matches
		if request[0] != cmd:
			return False

		# And has enough arguments
		req_len = len(request[1:])
		if not va and req_len != argc:
			return False
		elif va and req_len < argc:
			return False

		return True

	def send_response(self, request, remote, response_code, params = None):
		# Include status code, for example ["TXTUNE", "0", "941600"]
		request.insert(1, str(response_code))

		# Optionally append command specific parameters
		if params is not None:
			request += params

		# Add the response signature, and join back to string
		response = "RSP " + " ".join(request) + "\0"
		# Now we have something like "RSP TXTUNE 0 941600"
		self.sendto(response, remote)

	def parse_cmd(self, request):
		raise NotImplementedError
