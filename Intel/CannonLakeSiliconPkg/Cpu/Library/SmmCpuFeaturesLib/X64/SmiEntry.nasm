;; @file
;  Code template of the SMI handler for a particular processor
;
; @copyright
;  INTEL CONFIDENTIAL
;  Copyright 2016 - 2019 Intel Corporation.
;
;  The source code contained or described herein and all documents related to the
;  source code ("Material") are owned by Intel Corporation or its suppliers or
;  licensors. Title to the Material remains with Intel Corporation or its suppliers
;  and licensors. The Material may contain trade secrets and proprietary and
;  confidential information of Intel Corporation and its suppliers and licensors,
;  and is protected by worldwide copyright and trade secret laws and treaty
;  provisions. No part of the Material may be used, copied, reproduced, modified,
;  published, uploaded, posted, transmitted, distributed, or disclosed in any way
;  without Intel's prior express written permission.
;
;  No license under any patent, copyright, trade secret or other intellectual
;  property right is granted to or conferred upon you by disclosure or delivery
;  of the Materials, either expressly, by implication, inducement, estoppel or
;  otherwise. Any license under such intellectual property rights must be
;  express and approved by Intel in writing.
;
;  Unless otherwise agreed by Intel in writing, you may not remove or alter
;  this notice or any other notice embedded in Materials by Intel or
;  Intel's suppliers or licensors in any way.
;
;  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
;  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
;  the terms of your license agreement with Intel or your vendor. This file may
;  be modified by the user, subject to additional terms of the license agreement.
;
;@par Specification Reference:
;;


;
; Variables referrenced by C code
;

%define MSR_IA32_MISC_ENABLE 0x1A0
%define MSR_EFER 0xc0000080
%define MSR_EFER_XD 0x800

%define SMM_ENTRY_POINT_INFO_NULL               0x0
%define SMM_ENTRY_POINT_INFO_GDT                0x1
%define SMM_ENTRY_POINT_INFO_IDT                0x2
%define SMM_ENTRY_POINT_INFO_CR3                0x3
%define SMM_ENTRY_POINT_INFO_SUPOVR_STATE_LOCK  0x5
%define SMM_ENTRY_POINT_INFO_STACK              0x6
%define SMM_ENTRY_POINT_INFO_STACK_SIZE         0x7
%define SMM_ENTRY_POINT_INFO_END                0xFF

%define MSR_SMM_SUPOVR_STATE_LOCK          0x141
%define   MSR_SMM_SUPOVR_STATE_LOCK_PAGING_STATE_ENABLE_BIT  0x1
%define   MSR_SMM_SUPOVR_STATE_LOCK_SMBASE_ENABLE_BIT        0x2

%define PROTECT_MODE_CS 0x8
%define PROTECT_MODE_DS 0x20
%define LONG_MODE_CS    0x38
%define LONG_MODE_DS    0x20
%define TSS_SEGMENT     0x40

extern ASM_PFX(SmiRendezvous)
extern ASM_PFX(SmmFeatureCpuSmmDebugEntry)
extern ASM_PFX(SmmFeatureCpuSmmDebugExit)

SECTION .data

global ASM_PFX(gSmmFeatureSmiHandlerIdtr)
global ASM_PFX(gcSmmFeatureSmiHandlerTemplate)
global ASM_PFX(gPMStackDesc)
global ASM_PFX(gcSmmFeatureSmiHandlerSize)
global ASM_PFX(gSmmFeatureSmbase)
global ASM_PFX(gSmmFeatureSmiStack)
global ASM_PFX(gProtModeSmbase)
global ASM_PFX(gSmmFeatureSmiCr3)
global ASM_PFX(gSmmFeatureXdSupported)
global ASM_PFX(mSmmProtectedModeEnable)
global ASM_PFX(gcStmSmiHandlerOffset)
global ASM_PFX(gSmmSupovrStateLockData)
global ASM_PFX(gGdtDesc)
global ASM_PFX(gSmmStackSize)
global ASM_PFX(gSmmUsableStackSize)
global ASM_PFX(PpamSmiEntryPointEnd)

ASM_PFX(gSmmFeatureSmbase) EQU gSmmFeatureSmbasePatch - 4
ASM_PFX(gSmmFeatureSmiStack) EQU gSmmFeatureSmiStackPatch - 4
ASM_PFX(gProtModeSmbase) EQU gProtModeSmbasePatch - 4
ASM_PFX(gSmmFeatureSmiCr3) EQU gSmmFeatureSmiCr3Patch - 4
ASM_PFX(gSmmSupovrStateLockData) EQU gSmmSupovrStateLockDataPatch - 8
ASM_PFX(gSmmFeatureXdSupported) EQU gSmmFeatureXdSupportedPatch - 1

    DEFAULT REL
SECTION .text

BITS 16
ASM_PFX(gcSmmFeatureSmiHandlerTemplate):
_SmiEntryPoint:
    ;
    ; check real mode entry or protected mode entry
    ;
    mov     eax, cr0
    bt      ax, 0
    jc      _SmiPMEntryPoint

    ;
    ; real mode entry
    ;
    mov     bx, ASM_PFX(gGdtDesc) - _SmiEntryPoint + 0x8000  ; bx = GdtDesc offset
o32 lgdt    [cs:bx]                       ; lgdt fword ptr cs:[bx]
o32 mov     edi, strict dword 0
gSmmFeatureSmbasePatch:
    add     [cs:bx-0x6],edi
    mov     ebx, cr0
    and     ebx, 0x9ffafff3
    or      ebx, 0x23
    mov     cr0, ebx
    jmp     dword PROTECT_MODE_CS:(@ProtectedMode - _SmiEntryPoint) + 0x8000
ASM_PFX(gGdtDesc):
    DW 0
    DD 0

BITS 32
@ProtectedMode:
    mov     ax, PROTECT_MODE_DS
o16 mov     ds, ax
o16 mov     es, ax
o16 mov     fs, ax
o16 mov     gs, ax
o16 mov     ss, ax
    mov     esp, strict dword 0
gSmmFeatureSmiStackPatch:
    mov     eax, ASM_PFX(gGdtDesc) - _SmiEntryPoint + 0x8000
    add     eax, edi
    sub     [eax - 6], edi
    jmp     ProtFlatMode

BITS 64
;
; protected mode entry
;
_SmiPMEntryPoint:
; if SMM PROT MODE feature is ok, processor will break here with 32bit protected mode
    mov edi, strict dword 0
gProtModeSmbasePatch:
    ; reload SS:ESP, it should be done in one instruction
    mov     eax, ASM_PFX(gPMStackDesc) - _SmiEntryPoint + 0x8000
    add     eax, edi
    lss     esp, [rax]

    ; reload CS:EIP
    mov     eax, PMProtFlatMode - _SmiPMEntryPoint + 0x8000
    add     eax, edi
    add     [rax - 6], edi

    ; set cr0 value that is the same with real mode
    mov     rbx, cr0
    and     ebx, 0x9ffafff3
    or      ebx, 0x00000023
    mov     cr0, rbx

    DB      0xea                        ; jmp @ProtFlatMode
    DD      PMProtFlatMode - _SmiEntryPoint + 0x8000
    DW      PROTECT_MODE_CS
PMProtFlatMode:
    sub     [rax - 6], edi

ProtFlatMode:
    mov eax, strict dword 0
gSmmFeatureSmiCr3Patch:
    mov     cr3, rax
    mov     eax, 0x668                   ; as cr4.PGE is not set here, refresh cr3
    mov     cr4, rax                    ; in PreModifyMtrrs() to flush TLB.
; Load TSS
    sub     esp, 8                      ; reserve room in stack
    sgdt    [rsp]
    mov     eax, [rsp + 2]              ; eax = GDT base
    add     esp, 8
    mov     dl, 0x89
    mov     [rax + TSS_SEGMENT + 5], dl ; clear busy flag
    mov     eax, TSS_SEGMENT
    ltr     ax

; enable NXE if supported
    mov     al, strict byte 1
gSmmFeatureXdSupportedPatch:
    cmp     al, 0
    jz      @SkipXd
;
; Check XD disable bit
;
    mov     ecx, MSR_IA32_MISC_ENABLE
    rdmsr
    sub     esp, 4
    push    rdx                        ; save MSR_IA32_MISC_ENABLE[63-32]
    test    edx, BIT2                  ; MSR_IA32_MISC_ENABLE[34]
    jz      .0
    and     dx, 0xFFFB                 ; clear XD Disable bit if it is set
    wrmsr
.0:
    mov     ecx, MSR_EFER
    rdmsr
    or      ax, MSR_EFER_XD            ; enable NXE
    wrmsr
    jmp     @XdDone
@SkipXd:
    sub     esp, 8
@XdDone:

; Switch into @LongMode
    push    LONG_MODE_CS                ; push cs hardcore here
    call    Base                       ; push return address for retf later
Base:
    add     dword [rsp], @LongMode - Base; offset for far retf, seg is the 1st arg

    mov     ecx, MSR_EFER
    rdmsr
    or      ah, 1                      ; enable LME
    wrmsr
    mov     rbx, cr0
    or      ebx, 0x80010023            ; enable paging + WP + NE + MP + PE
    mov     cr0, rbx
    retf
@LongMode:                             ; long mode (64-bit code) starts here
    mov     rax, strict qword 0
gSmmFeatureSmiHandlerIdtrAbsAddr:      ; mov     rax, ASM_PFX(gSmmFeatureSmiHandlerIdtr) - _SmiEntryPoint + 0x8000
    add     rax, rdi
    lidt    [rax]
    mov     ax, LONG_MODE_DS
    mov     ds, eax
    mov     ax, LONG_MODE_DS
    mov     es, eax
    mov     fs, eax
    mov     gs, eax
    mov     ax, LONG_MODE_DS
    mov     ss, eax
    mov     rax, strict qword 0
CommonHandlerAbsAddr:
    jmp     rax
CommonHandler:

@SmmSupovrStateLock:
    mov     rax, strict qword 0
gSmmSupovrStateLockDataPatch:
    cmp     rax, 0
    jz      @SmmSupovrStateLockDone
    mov     ecx, MSR_SMM_SUPOVR_STATE_LOCK
    mov     rdx, rax
    shr     rdx, 0x20                   ; edx:eax contains the value to write
    wrmsr
    jmp     @SmmSupovrStateLockDone
@SmmSupovrStateLockDone:

    jmp     ASM_PFX(PpamSmiEntryPointEnd)

align 16
; used by LSS
ASM_PFX(gPMStackDesc):
    DD 0
    DD PROTECT_MODE_DS

align 16
ASM_PFX(gSmmFeatureSmiHandlerIdtr):
    DW 0
    DQ 0

ASM_PFX(gSmmStackSize):      DD 0x0

ASM_PFX(PpamSmiEntryPointEnd):

    mov     rbx, [rsp + 0x8]             ; rbx <- CpuIndex

    ;
    ; Save FP registers
    ;
    sub     rsp, 0x200
    fxsave64  [rsp]                      ; FXSAVE64

    add     rsp, -0x20

    mov     rcx, rbx
    call    ASM_PFX(SmmFeatureCpuSmmDebugEntry)

    mov     rcx, rbx
    call    ASM_PFX(SmiRendezvous)  ; rax <- absolute addr of SmiRedezvous

    mov     rcx, rbx
    call    ASM_PFX(SmmFeatureCpuSmmDebugExit)

    add     rsp, 0x20

    ;
    ; Restore FP registers
    ;
    fxrstor64 [rsp]                      ; FXRSTOR64

    add     rsp, 0x200

    ;
    ; Clear stack
    ;
    lea     rax, [ASM_PFX(gSmmUsableStackSize)]
    xor     rbx, rbx
    mov     ebx, [rax]
    lea     rax, [rsp]
    add     rax, 0x1000
    and     rax, 0xFFFFF000
    sub     rax, rbx
    lea     rcx, [rsp]
    sub     rcx, rax
    lea     rdi, [rsp - 1]
    xor     rax, rax
    std
    rep stosb
    cld

    lea     rax, [ASM_PFX(gSmmFeatureXdSupported)]
    mov     al, [rax]
    cmp     al, 0
    jz      .1
    pop     rdx                       ; get saved MSR_IA32_MISC_ENABLE[63-32]
    test    edx, BIT2
    jz      .1
    mov     ecx, MSR_IA32_MISC_ENABLE
    rdmsr
    or      dx, BIT2                  ; set XD Disable bit if it was set before entering into SMM
    wrmsr

.1:
    rsm

align 16
ASM_PFX(gSmmUsableStackSize):   DD 0x0

_StmSmiHandler:
;
; Check XD disable bit
;
    xor     r8, r8
    lea     rax, [ASM_PFX(gSmmFeatureXdSupported)]
    mov     al, [rax]
    cmp     al, 0
    jz      @StmXdDone
    mov     ecx, MSR_IA32_MISC_ENABLE
    rdmsr
    mov     r8, rdx                    ; save MSR_IA32_MISC_ENABLE[63-32]
    test    edx, BIT2                  ; MSR_IA32_MISC_ENABLE[34]
    jz      .0
    and     dx, 0xFFFB                 ; clear XD Disable bit if it is set
    wrmsr
.0:
    mov     ecx, MSR_EFER
    rdmsr
    or      ax, MSR_EFER_XD            ; enable NXE
    wrmsr
@StmXdDone:
    push    r8

    ; below step is needed, because STM does not run above code.
    ; we have to run below code to set IDT/CR0/CR4

    lea     rax, [ASM_PFX(gSmmFeatureSmiHandlerIdtr)]
    lidt    [rax]

    mov     rax, cr0
    or      eax, 0x80010023            ; enable paging + WP + NE + MP + PE
    mov     cr0, rax
    mov     rax, cr4
    mov     eax, 0x668                 ; as cr4.PGE is not set here, refresh cr3
    mov     cr4, rax                   ; in PreModifyMtrrs() to flush TLB.
    ; STM init finish
    jmp     CommonHandler

ASM_PFX(gcStmSmiHandlerOffset):      DW _StmSmiHandler - _SmiEntryPoint
ASM_PFX(gcSmmFeatureSmiHandlerSize): DW _EntryPointEnd - _SmiEntryPoint
_EntryPointEnd:

global ASM_PFX(SmmCpuFeaturesLibSmiEntryFixupAddress)
ASM_PFX(SmmCpuFeaturesLibSmiEntryFixupAddress):
    lea    rax, [(ASM_PFX(gSmmFeatureSmiHandlerIdtr) - _SmiEntryPoint) + 0x8000]; ASM_PFX(gSmmFeatureSmiHandlerIdtr) - _SmiEntryPoint + 0x8000
    lea    rcx, [gSmmFeatureSmiHandlerIdtrAbsAddr]
    mov    qword [rcx - 8], rax
    ;
    lea    rax, [CommonHandler]
    lea    rcx, [CommonHandlerAbsAddr]
    mov    qword [rcx - 8], rax
    ret

