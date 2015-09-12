# -*- coding: UTF-8 -*-

################################################################################
#
# ARTX test script
#
################################################################################
#
# ARTX - A realtime executive library for Atmel AVR microcontrollers
#
# Copyright (C) 2007-2015 Marcus Holland-Moritz.
#
# ARTX is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ARTX is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ARTX.  If not, see <http://www.gnu.org/licenses/>.
#
################################################################################

from bisect import bisect_right
from collections import defaultdict
from elftools.elf.elffile import ELFFile, SymbolTableSection
from operator import itemgetter
from unittest import TestSuite, TextTestRunner, TestCase, defaultTestLoader, main
from sys import argv, stderr

import os
import pysimulavr
import re
import subprocess

class SymbolTable(object):
    def __init__(self, elf):
        self.__glb = defaultdict(dict)
        self.__loc = defaultdict(lambda : defaultdict(dict))
        self.__map = []
        cur_file = ''
        with open(elf, 'rb') as f:
            ef = ELFFile(f)
            for section in ef.iter_sections():
                if isinstance(section, SymbolTableSection):
                    for symbol in section.iter_symbols():
                        if symbol['st_other']['visibility'] == 'STV_DEFAULT':
                            stype = symbol['st_info']['type'][4:].lower()
                            sbind = symbol['st_info']['bind']
                            if stype == 'file':
                                cur_file = symbol.name
                            elif stype != 'notype':
                                scope = ''
                                if sbind == 'STB_LOCAL':
                                    scope = cur_file + ':'
                                    self.__loc[cur_file][stype][symbol.name] = symbol['st_value']
                                elif sbind == 'STB_GLOBAL':
                                    self.__glb[stype][symbol.name] = symbol['st_value']
                                self.__map.append([
                                    symbol['st_value'], symbol['st_size'],
                                    '{0}{1}'.format(scope, symbol.name)])
        self.__map.sort(key=itemgetter(0))
        self.__idx = [i[0] for i in self.__map]

    def __call__(self, name, stype='func', scope=None):
        where = self.__glb if scope is None else self.__loc.get(scope)
        if where is None:
            return None
        return where.get(stype, {}).get(name)

    def addr2sym(self, addr):
        i = bisect_right(self.__idx, addr)
        if i:
            m = self.__map[i-1]
            off = addr - m[0]
            if off == 0:
                return m[2]
            elif off < m[1]:
                return '{0}+{1}'.format(m[2], off)
        return str(addr)

class SimulavrAdapter(object):
    DEFAULT_CLOCK_SETTING = 1000 # 1000ns or 1MHz

    def loadDevice(self, t, e):
        self.__sc = pysimulavr.SystemClock.Instance()
        self.__sc.ResetClock()
        dev = pysimulavr.AvrFactory.instance().makeDevice(t)
        dev.Load(e)
        dev.SetClockFreq(self.DEFAULT_CLOCK_SETTING)
        self.__sc.Add(dev)
        return dev

    def clock(self):
        return self.__sc

class Breakpoint(object):
    def __init__(self, name, addr, dev, clock):
        self.name = name
        self.addr = addr
        self.__dev = dev
        self.__clock = clock

    def leave(self):
        self.__dev.BP.RemoveBreakpoint(self.addr)
        while self.__dev.PC == self.addr:
            ret = self.__clock.Step()
            assert ret == 0
        self.__dev.BP.AddBreakpoint(self.addr)

class DeviceBase(object):
    __TARGET__ = 'artxtest'

    @classmethod
    def target(cls):
        return '{0}_{1}'.format(cls.__TARGET__, cls.DEVICE)

    @classmethod
    def target_elf(cls):
        return '{0}.elf'.format(cls.target())

    @classmethod
    def build_target(cls, *args):
        subprocess.check_output([
            'make',
            'TARGET={0}'.format(cls.target()),
            'MCU={0}'.format(cls.DEVICE),
        ] + list(args), stderr=subprocess.STDOUT)

class DeviceMega16(DeviceBase):
    DEVICE = 'atmega16'
    VECTOR = '__vector_6'

class DeviceTiny85(DeviceBase):
    DEVICE = 'attiny85'
    VECTOR = '__vector_3'

class TestBaseClass(TestCase, SimulavrAdapter):
    def setUp(self):
        self.symtab = SymbolTable(self.target_elf())
        self.device = self.loadDevice(self.DEVICE, self.target_elf())
        self.bpaddr = {}
        self.bpname = {}
        self.sp = {}

    def tearDown(self):
        del self.device

    def break_at(self, name, scope=None):
        # addr = self.device.Flash.GetAddressAtSymbol(name)
        addr = self.symtab(name, scope=scope) >> 1
        self.bpaddr[name] = addr
        self.bpname[addr] = name
        self.device.BP.AddBreakpoint(addr)

    def delete(self, name):
        addr = self.bpaddr.pop(name)
        self.bpname.pop(addr)
        self.device.BP.RemoveBreakpoint(addr)

    def get_return_addr(self):
        ## TODO: stack ptr always at 0x5d ?
        return 2*self.addr2word(self.addr2word(0x5d))

    def run_debug(self, n):
        clock = self.clock()
        ct = clock.GetCurrentTime
        te = ct() + n
        while ct() < te:
            res = clock.Step()
            # debug = False
            # if ct() >= 5800000000 and ct() <= 6300000000:
            #     for task in ['intr', 'ut0', 'ut1', 'ut2', 'ut3', 'idle']:
            #         sp = self.read_word(task, 'artxtest.c')
            #         old_sp = self.sp.get(task)
            #         if old_sp is not None and sp != old_sp:
            #             stderr.write("{0}: {1} sp {2:x} -> {3:x}\n".format(ct(), task, old_sp, sp))
            #             debug = True
            #         self.sp[task] = sp
            # if debug or (ct() >= 2834000 and ct() < 12105000) or \
            #         (ct() >= 5900000000 and ct() <= 6207141000):
            #     tcb = self.read_word('artx_current_tcb', 'task.c')
            #     tsp = self.addr2word(tcb)
            #     schedule = self.addr2word(tcb + 6)
            #     sreg = self.device.getRWMem(0x5f)
            #     sp = self.addr2word(0x5d)
            #     top = self.addr2word(sp + 2)
            #     if schedule > 32767:
            #         schedule -= 65536
            #     stderr.write("{0}: {1} (0x{2:04x}) - tcb@{3:04x} ({4}), SREG={5:08b}, SP={6:04x}, TOP={7:04x}/{8:04x}: tsp={9:x}, schedule={10}\n".format(
            #         ct(), self.symtab.addr2sym(self.device.PC << 1), self.device.PC << 1,
            #         tcb, self.symtab.addr2sym(0x800000 | tcb), sreg, sp, top, 2*top, tsp, schedule))
            if res is not 0: return res
        return 0

    def cont(self, ns=1000000000):
        ret = self.run_debug(ns)
        if ret != -2:
            return None
        addr = self.device.PC
        return Breakpoint(self.bpname[addr], addr, self.device, self.clock())

    def addr2word(self, addr):
        d1 = self.device.getRWMem(addr + 1)
        d2 = self.device.getRWMem(addr)
        return d2 + (d1 << 8)

    def read_word(self, name, scope=None):
        addr = self.symtab(name, stype='object', scope=scope)
        return self.addr2word(addr & 0xfffff) if addr is not None else None

    def test_consistency(self):
        name = 'main'
        addr1 = self.device.Flash.GetAddressAtSymbol(name)
        addr2 = self.symtab(name) >> 1
        self.assertEqual(addr1, addr2)

    def test_basic_scheduling(self):
        "basic scheduling logic"
        routines = [
            'run_intr',
            'run_ut0',
            'run_ut1',
            'run_ut2',
            'run_ut3',
            'background',
        ]
        timebase = {
            # routine: (interval, max_jitter)
            self.VECTOR: (2, 0.5),
            'run_intr': (2, 1),
            'run_ut0': (8, 1.5),
            'run_ut1': (50, 2),
            'run_ut2': (32, 3),
            'run_ut3': (64, 4),
        }

        for r in routines:
            self.break_at(r, scope='artxtest.c')

        self.break_at('main')
        self.break_at(self.VECTOR)
        self.break_at('ARTX_schedule')

        bp = self.cont()
        self.assertEqual(bp.name, 'main')
        bp.leave()

        bp = self.cont()
        self.assertEqual(bp.name, 'ARTX_schedule')
        bp.leave()

        self.delete('ARTX_schedule')

        get_ct = self.clock().GetCurrentTime
        t_start = get_ct()

        # Run for 1 second
        t_end = t_start + 1e9
        hits = []
        cont = set()
        while get_ct() <= t_end:
            bp = self.cont()
            # stderr.write("{0} {1} {2}\n".format(get_ct(), bp.name, self.read_word('artx_current_tcb', 'task.c')))
            if bp.name == self.VECTOR:
                # timer interrupt; if the interrupt occurred exactly when we just
                # started a new task, take a note here so that we don't count twice
                raddr = self.get_return_addr()
                rname = self.symtab.addr2sym(raddr).split(':')[-1]
                if re.match('\w+$', rname):
                    cont.add(rname)
                    # stderr.write("susp: {0} ({1:x})\n".format(rname, raddr))
            if bp.name in cont:
                # stderr.write("cont: {0}\n".format(bp.name))
                cont.remove(bp.name)
            else:
                hits.append((bp.name, get_ct()))
            bp.leave()

        self.assertGreater(len(hits), 0)

        tbvec = timebase[self.VECTOR][0]
        tstart = min([1e-6*t - n*tbvec for n, t in enumerate(
                        [h[1] for h in hits if h[0] == self.VECTOR])])

        bg_is_last = None
        delta = defaultdict(list)
        for h in hits:
            if h[0] == 'background':
                if h[0] not in delta:
                    bg_is_last = set(delta.keys()) == (set(timebase.keys()))
            else:
                d = delta[h[0]]
                val = 1e-6*h[1] - (tstart + len(d)*timebase[h[0]][0])
                d.append(val)

        for k, d in delta.iteritems():
            # stderr.write("{0}: {1:.2f} {2:.2f} {3:.2f}\n".format(k, min(d), max(d), (sum(d)/len(d))))
            self.assertGreaterEqual(min(d), 0, k)
            self.assertLess(min(d), timebase[k][1], k)

        self.assertTrue(bg_is_last)

class TestMega16(TestBaseClass, DeviceMega16):
    pass

class TestTiny85(TestBaseClass, DeviceTiny85):
    pass

if __name__ == "__main__":
  classes = [TestMega16, TestTiny85]
  allTestsFrom = defaultTestLoader.loadTestsFromTestCase
  suite = TestSuite()
  for cls in classes:
      cls.build_target('clean')
      cls.build_target()
      suite.addTests(allTestsFrom(cls))
  TextTestRunner(verbosity = 0).run(suite)
  for cls in classes:
      cls.build_target('realclean')
