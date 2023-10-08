#include <Windows.h>
#include <wlanapi.h>
#include <string>
#include <iostream>
#include <tchar.h>
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"wlanapi.lib")

using namespace std;

class cFuncoes
{
private:

	HANDLE IdentificadorWLAN;
	bool InicializarIdentificadorWLAN()
	{
		bool bResult = false;

		DWORD Res;
		DWORD NegotiateVersion;
		Res = WlanOpenHandle(WLAN_API_VERSION_2_0, 0, &NegotiateVersion, &IdentificadorWLAN);
		if (Res == ERROR_SUCCESS)
		{
			bResult = true;
		}
		else if (Res == ERROR_NOT_ENOUGH_MEMORY)
		{
			cout << "N�o foi poss�vel alocar a mem�ria para o contexto do cliente.";
		}

		return bResult;
	}

	WLAN_INTERFACE_INFO_LIST* Lista;
	DWORD DataSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
	WLAN_CONNECTION_ATTRIBUTES* Atributos;

	WLAN_AVAILABLE_NETWORK_LIST* Encontrados;

public:

	void PesquisarRedesDisponiveis()
	{
		WlanGetAvailableNetworkList(IdentificadorWLAN, &Lista->InterfaceInfo->InterfaceGuid,
			WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES, 0, &Encontrados);

		cout << "Dispositivos encontrados:\n\n";

		for (int i = 0; i < Encontrados->dwNumberOfItems; i++)
		{
			cout << "Wi-Fi: " << Encontrados->Network[i].dot11Ssid.ucSSID << '\n';
			cout << "Qualidade do sinal: " << Encontrados->Network[i].wlanSignalQuality << "% \n";
			if (Encontrados->Network[i].bSecurityEnabled)
				cout << "Conexao protegida.\n\n";
			else
				cout << "Conexao aberta.\n\n";
		}
	}

	//Esta interface sem fio se refere a rede conectada a este computador.
	void ObtendoInformacoesDeInterface()
	{
		if (InicializarIdentificadorWLAN() == true)
		{
			DWORD Res;
			Res = WlanEnumInterfaces(IdentificadorWLAN, 0, &Lista);
			if (Res == ERROR_SUCCESS)
			{
				for (int i = 0; i < Lista->dwNumberOfItems; i++)
				{
					Res = WlanQueryInterface(IdentificadorWLAN, &Lista[i].InterfaceInfo->InterfaceGuid,
						wlan_intf_opcode_current_connection, 0, &DataSize, (PVOID*)&Atributos, 0);
					if (Res == ERROR_SUCCESS)
					{
						cout << "Obtendo informa��es da conex�o atual...\n";
						cout << "Nome da rede: " << Atributos->wlanAssociationAttributes.dot11Ssid.ucSSID << '\n';
						cout << "Lat�ncia de download: " << Atributos->wlanAssociationAttributes.ulRxRate << " KBps \n";
						cout << "Lat�ncia de upload: " << Atributos->wlanAssociationAttributes.ulTxRate << " KBps \n";

						cout << "Calculando as lat�ncias para obtermos os valores em MBps\n";

						//Dividimos a lat�ncia de cada valor por 8.
						int Divisor = 8;
						long Download = (double)Atributos->wlanAssociationAttributes.ulRxRate / Divisor;
						long Upload = (double)Atributos->wlanAssociationAttributes.ulTxRate / Divisor;

						cout << "Velocidade de download: " << Download << " MBps\n";
						cout << "Velocidade de upload: " << Upload << " MBps\n";

						cout << "Qualidade do sinal: " << Atributos->wlanAssociationAttributes.wlanSignalQuality << "% \n\n";

						cout << "Analisando seguran�a:\n";
						if (Atributos->wlanSecurityAttributes.bSecurityEnabled)
							cout << "Esta conex�o � segura.\n";
						else
							cout << "Esta conex�o n�o � segura.\n";

						if (Atributos->wlanSecurityAttributes.bOneXEnabled)
							cout << "O protocolo de seguran�a IEEE 802.1X est� habilitado.\n";
						else
							cout << "O protocolo de seguran�a IEEE 802.1X n�o est� habilitado.\n";

						cout << "Verificando algoritmos de autentica��o de seguran�a:\n\n";

						switch (Atributos->wlanSecurityAttributes.dot11AuthAlgorithm)
						{
						case DOT11_AUTH_ALGO_80211_OPEN:
							cout << "Algoritmo de sistema aberto 802.11.";
							break;
						case DOT11_AUTH_ALGO_80211_SHARED_KEY:
							cout << "Algoritmo de chave compartilhada 802.11, requer o uso de chave WEP.";
							break;
						case DOT11_AUTH_ALGO_WPA:
							cout << "Algoritmo de Wi-Fi Protected Access ( WPA ), mais comum.";
							break;
						case DOT11_AUTH_ALGO_WPA_PSK:
							cout << "Algoritmo de Wi-Fi Protected Access ( WPA ) pr�-compartilhado.";
							break;
						case DOT11_AUTH_ALGO_RSNA:
							cout << "Algoritmo de Wi-Fi Robust Security Network Association (RSNA) 802.11i.";
							break;
						case DOT11_AUTH_ALGO_RSNA_PSK:
							cout << "Algoritmo de Wi-Fi Robust Security Network Association (RSNA) 802.11i que usa PSK.";
							break;
						case DOT11_AUTH_ALGO_WPA3_SAE:
							cout << "Algoritmo de autentica��o simultanea WPA3-SAE.";
							break;
						case DOT11_AUTH_ALGO_WPA3_ENT:
							cout << "Algoritmo de autentica��o WPA3-Enterprise.";
							break;

							//Voc� poder� colocar outros nesta cadeia.

						default:
							//Outro.
							break;
						}

						cout << "\n";
					}
					else
					{
						cout << "N�o foi poss�vel prosseguir com a solicita��o..\n" << GetLastError();
					}
				}
			}
			else
			{
				cout << "N�o foi poss�vel enumerar interface de rede..\n";
			}
		}
	}

	void DesconectarWiFi()
	{
		//Ir� desconectar usando o identificador atual obtido.
		WlanDisconnect(IdentificadorWLAN, &Lista->InterfaceInfo->InterfaceGuid, 0);

		WlanFreeMemory(Lista);

		//Iremos finalizar o identificador, pois n�o ser� mais necess�rio.
		WlanCloseHandle(IdentificadorWLAN, 0);
	}

}Funcoes;

int main()
{

	cout << "O assistente est� executando pesquisas e opera��es para dispositivos Wi-Fi...";

	Funcoes.ObtendoInformacoesDeInterface();
	Funcoes.PesquisarRedesDisponiveis();
	Funcoes.DesconectarWiFi();

	system("pause");
}