////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorStub.h"
#include "IDirect3DVolume9InterceptorStub.h"
#include "IDirect3DVolumeTexture9InterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3DVolumeTexture9InterceptorStub::IDirect3DVolumeTexture9InterceptorStub(DXPainter* painter, IDirect3DVolumeTexture9* original, IDirect3DDevice9InterceptorStub* creator) : 
DXInterceptorStub(painter),
m_original(original),
m_creator(creator)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();

  DWORD levels = m_original->GetLevelCount();
  m_locks = new DXVolumeLock[levels];

  // Reset the internal objects
  for (unsigned int i=0; i < levels; ++i)
  {
    IDirect3DVolume9* volume;
    if (SUCCEEDED(m_original->GetVolumeLevel(i, &volume)))
    {
      volume->FreePrivateData(IID_IReferenceCounter);
      volume->Release();
    }
  }
  
  D3DVOLUME_DESC	description;
  m_original->GetLevelDesc(0, &description);
  m_format = description.Format;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVolumeTexture9InterceptorStub::~IDirect3DVolumeTexture9InterceptorStub()
{
  if (m_locks)
  {
    delete[] m_locks;
    m_locks = NULL;
  }

  FREE_D3D_RESOURCE();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolumeTexture9InterceptorStub::DoSpecific(DXMethodCallPtr call)
{
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_QueryInterface:
    {
      IID param1;
      CHECK_CALL(call->Pop_IID(&param1));

      DXIgnoredParameter ignoredParam;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&ignoredParam));
      
      VOID* param2;

      HRESULT hr = m_original->QueryInterface(param1, &param2);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_AddRef:
    {
      ULONG result = m_original->AddRef();
      CHECK_CALL_RETURN_VALUE(ULONG, result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_Release:
    {
      ULONG result = m_original->Release();
      if (!result)
      {
        m_painter->RemoveStub(this);
        m_original = NULL;
      }
      CHECK_CALL_RETURN_VALUE_ADDREF_RELEASE(ULONG, result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_GetDevice:
    {
      DXResourceObjectID param1ResID;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      IDirect3DDevice9* param1;

      HRESULT hr = m_original->GetDevice(&param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_GetVolumeLevel:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));
      
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param2ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
      }
      IDirect3DVolume9* pVolume;

      HRESULT hr = m_original->GetVolumeLevel(param1, &pVolume);

      if (FAILED(hr))
      {
        CHECK_CALL_RETURN_VALUE_HRESULT(hr);
      }

      IReferenceCounter* pRefCounter;
      DWORD pRefCounterSize = sizeof(IUnknown*);
      
      if (FAILED(pVolume->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
      {
        new IDirect3DVolume9InterceptorStub(m_painter, param1, pVolume, m_creator);
      }
      else
      {
        pRefCounter->Release();
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_LockBox:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));
      
      DXIgnoredParameter param2;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param2));

      D3DBOX param3;
      D3DBOX* param3Ptr = &param3;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param3Ptr = NULL;
        }
        else
        {
          CHECK_CALL(call->Pop_D3DBOX(param3Ptr));
        }
      }

      DXFlagsD3DLOCK param4;
      CHECK_CALL(call->Pop_DXFlagsD3DLOCK(&param4));

      D3DLOCKED_BOX lockedBox;

      HRESULT hr = m_original->LockBox(param1, &lockedBox, param3Ptr, param4);

      if (FAILED(hr) || (param4 & D3DLOCK_READONLY))
      {
        lockedBox.pBits = 0;
        m_locks[param1].SetLockedBox(&lockedBox);
      }
      else
      {
        m_locks[param1].SetLockedBox(&lockedBox);
        m_locks[param1].SetLock(m_original, param1, param3Ptr, m_format);
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_UnlockBox:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));
      
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXTextureIdentifier texture_id;
        CHECK_CALL(call->Pop_DXTextureIdentifier(&texture_id));
        DXTexturePtr texture;
        CHECK_CALL(m_painter->GetTexture(&texture, texture_id));

        if (!m_locks[param1].ReadLock(texture))
        {
          return E_FAIL;
        }
      }

      HRESULT hr = m_original->UnlockBox(param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_GetPrivateData:
    return D3DResource9_GetPrivateData(m_original, call);
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_SetPrivateData:
    return D3DResource9_SetPrivateData(m_original, call);
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolumeTexture9_FreePrivateData:
    return D3DResource9_FreePrivateData(m_original, call);
    break;
  }

  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////
