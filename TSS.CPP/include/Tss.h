/*
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See the LICENSE file in the project root for full license information.
 */
#pragma once

#include "TpmTypes.h"

_TPMCPP_BEGIN

// Convenience aliases
using TPM_ALG = TPM_ALG_ID;
using TPM_HASH = TPMT_HA;

constexpr auto TPM_ALG_NULL = TPM_ALG_ID::_NULL;
constexpr auto TPM_RH_NULL = TPM_RH::_NULL;

_TPMCPP_END

#include "TpmPolicy.h"
#include "Crypto.h"


_TPMCPP_BEGIN

// TODO: Better encapsulation and better constructors

///<summary>AUTH_SESSION encapsulates state + methods for TPM authorization sessions.</summary>
class _DLLEXP_ AUTH_SESSION
{
        friend class Tpm2;
    public:
        AUTH_SESSION() {}
                     
        AUTH_SESSION(const TPM_HANDLE& sessionHandle, TPM_SE type, TPM_ALG_ID hashAlg,           
                     const ByteVec& nonceCaller, const ByteVec& nonceTpm,   
                     TPMA_SESSION attributes,
                     const TPMT_SYM_DEF& symDef,       // OPT
                     const ByteVec& salt,              // OPT
                     const TPM_HANDLE& boundObject);   // OPT

        static AUTH_SESSION PWAP()
        {
            AUTH_SESSION s;
            s.PWap = true;
            s.handle = TPM_RH::PW;
            return s;
        }

        ///<summary>Casting operator so that sessions can be used in place of handles</summary>
        operator const TPM_HANDLE& () const { return handle; }

        bool IsPWAP() const { return PWap; }

        void IncludePlaintextPassword() { IncludePlaintextPasswordInPolicySession = true; }

        void SetSessionIncludesAuth() { SessionIncludesAuth = true; }

        void ForceHmac() { ForceHmacOnPolicySession = true; }

        TPM_ALG_ID GetHashAlg() { return HashAlg; }

        ByteVec GetNonceTpm() { return NonceTpm; }

    protected:
        void Init();
        void CalcSessionKey();

        ByteVec ParmEncrypt(ByteVec& parm, bool direction /* false == response */);

        bool CanEncrypt();

        bool HasSymmetricCipher() {

            return (Symmetric.algorithm != TPM_ALG_ID::_NULL);
        }

        ByteVec GetAuthHmac(const ByteVec& parmHash, bool directionIn,
                            const ByteVec& nonceDec, const ByteVec& nonceEnc,
                            const TPM_HANDLE *associatedHandle);

        bool SessionInitted = false;

        ///<summary>Set the auth-value to be used for the next use of this session.</summary>
        void SetAuthValue(const ByteVec& _authVal) { AuthValue = _authVal; }

        ///<summary>Session handle</summary>
        TPM_HANDLE handle;

        ///<summary>Type of session (HMAC, policly or trial-policy).</summary>
        TPM_SE SessionType;

        ///<summary>Most recent nonce returned by TPM.</summary>
        ByteVec NonceTpm;

        ///<summary>TSS.C++ library nonce.</summary>
        ByteVec NonceCaller;

        ///<summary>Session hash algorithm (HMAC or policy-hash).</summary>
        TPM_ALG_ID HashAlg;

        ///<summary>Attribues</summary>
        TPMA_SESSION SessionAttributes;

        ///<summary>Algorithm-info for encrypt/decrypt sessions.</summary>
        TPMT_SYM_DEF Symmetric;

        ByteVec SessionKey;
        ByteVec AuthValue;
        ByteVec Salt;

        ///<summary>Object to which the session is bound (needed for AuthValue).</summary>
        TPM_HANDLE BindObject;

        bool PWap = false;
        bool SessionIncludesAuth = false;
        bool ForceHmacOnPolicySession = false;
        bool IncludePlaintextPasswordInPolicySession = false;

};
///<summary>This class encapsulates the data needed to call Activate().</summary>
class ActivationData {
    public:
        TPMS_ID_OBJECT CredentialBlob;
    public:
        ByteVec Secret;
};

///<summary>This class encapsulates the data you need to Import a key.</summary>
class DuplicationBlob {    
    public:
        ///<summary>The optional symmetric encryption key used as the inner wrapper for
        /// duplicate. If symmetricAlg is TPM_ALG_NULL, then this parameter shall be the
        /// Empty Buffer</summary>
        ByteVec EncryptionKey;

        ///<summary>The symmetrically encrypted duplicate object that may contain an inner
        /// symmetric wrapper</summary>
        ByteVec DuplicateObject;

        ///<summary>Symmetric key used to encrypt duplicate inSymSeed is
        /// encrypted / encoded using the algorithms of newParent.</summary>
        ByteVec EncryptedSeed;

        ///<summary>Set to random key used for inner-wrapper (if an inner-wrapper is requested).</summary>
        ByteVec InnerWrapperKey;

        operator TPM2B_PRIVATE () const { return DuplicateObject; }
};

_TPMCPP_END