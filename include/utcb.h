/*
 * User Thread Control Block (UTCB)
 *
 * Copyright (C) 2007-2009, Udo Steinberg <udo@hypervisor.org>
 *
 * This file is part of the NOVA microhypervisor.
 *
 * NOVA is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NOVA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License version 2 for more details.
 */

#pragma once

#include "buddy.h"
#include "compiler.h"
#include "crd.h"
#include "mtd.h"
#include "types.h"

class Exc_regs;

class Utcb_segment
{
    public:
        uint16  sel;
        uint16  ar;
        uint32  limit;
        union {
            uint64  : 64;
            mword   base;
        };

        ALWAYS_INLINE
        inline void set_vmx (mword s, mword b, mword l, mword a)
        {
            sel   = static_cast<uint16>(s);
            ar    = static_cast<uint16>((a >> 4 & 0x1f00) | (a & 0xff));
            limit = static_cast<uint32>(l);
            base  = static_cast<uint64>(b);
        }
};

class Utcb
{
    public:
        mword   pid;
        Mtd     mtr;
        Crd     crd;
        mword   res[2];
        mword   tls;

    private:
        union {
            struct {
                mword           rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi;
                mword           rflags, rip;
                mword           cr0, cr2, cr3, cr4, cr8, dr7;
                Utcb_segment    es, cs, ss, ds, fs, gs, ld, tr, gd, id;
                union {
                    uint64      inj;
                    struct {
                        uint32  intr_info, intr_error;
                    };
                };
                uint32          intr_state, actv_state;
                uint64          qual[2];
                uint32          ctrl[2];
                union {
                    uint64      tsc;
                    struct {
                        uint32  tsc_lo;
                        uint32  tsc_hi;
                    };
                };
                mword           inst_len;
                mword           sysenter_cs, sysenter_rsp, sysenter_rip;
            };
            mword mr[];
        };
        mword item[];

    public:
        ALWAYS_INLINE
        static inline void *operator new (size_t)
        {
            return Buddy::allocator.alloc (0, Buddy::FILL_0);
        }

        ALWAYS_INLINE
        static inline void operator delete (void *ptr)
        {
            Buddy::allocator.free (reinterpret_cast<mword>(ptr));
        }

        ALWAYS_INLINE NONNULL
        inline mword *save (Utcb *dst, unsigned long untyped)
        {
            dst->mtr = Mtd (untyped);
#if 0
            mword *d = dst->mr, *s = mr;
            asm volatile ("rep; movsl" : "+D" (d), "+S" (s), "+c" (untyped) : : "memory");
#else
            for (unsigned long i = 0; i < untyped; i++)
                dst->mr[i] = mr[i];
#endif
            return mr + untyped;
        }

        void    load_exc (Exc_regs *, Mtd);
        mword * save_exc (Exc_regs *, Mtd);

        void    load_vmx (Exc_regs *, Mtd);
        mword * save_vmx (Exc_regs *, Mtd);

        void    load_svm (Exc_regs *, Mtd);
        mword * save_svm (Exc_regs *, Mtd);
};