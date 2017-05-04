/************************************************************
NKFADC500 GUI system
Made by Byul Moon.
FADC500run.cc source file
Run NKFADC500 DAQ and save data.
May. 1. 2017.
***********************************************************/

#include "TROOT.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "TCanvas.h"
#include "TH1F.h"
#include "FADC500run.h"
#include <vector>
#include "TSystem.h"
#include "TString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "usb3tcbroot.h"
#include "usb3comroot.h"
#include "NoticeTCBIBSROOT.h"
#include "NoticeFADC500IBSROOT.h"

using namespace std;

#define PC_DRAM_SIZE      10                           // available PC DRAM size in Mbyte
#define DATA_ARRAY_SIZE   PC_DRAM_SIZE*1024*1024       // array size in byte
#define CHUNK_SIZE        PC_DRAM_SIZE*1024            // array size in kilobyte


int FADC500run::FADC500DAQRun(TString ifilename, int nEvent, int nModule)
{
	const int nMod = nModule;
	vector <int> sid;
	for (i = 0; i < nMod; i++)
	{
		sid.push_back(i+1);
		printf("Serial ID : %d\n", sid[i]);
	}

	gSystem->ProcessEvents();
	gSystem->Load("libusb3comroot.so");           // load usb3 library
	gSystem->Load("libNoticeFADC500IBSROOT.so");  // load fadc500ibs library

	usb3comroot *usb = new usb3comroot;
	usb->USB3Init(0);

	NKFADC500IBS *fadc = new NKFADC500IBS;

	for(i = 0; i< nMod; i++)
	{
		fadc->FADC500IBSopen(sid[i], 0);
	}

	TString filename = ifilename;
	TObjArray *decomposedFileNameWithPath = ifilename.Tokenize(".");
	TString filetype = ((TObjString *) decomposedFileNameWithPath -> Last()) -> GetString();
	TString lfilename = ifilename.ReplaceAll(filetype, "log");

	int numfile = 1;

	fp = fopen(filename, "wb");
	lfp = fopen(lfilename, "wt");


	flag = 1;
	adcflag = 1;
	tdcflag = 1;
	iEvent = 0;

	while (1)
	{
		gSystem->ProcessEvents();
		for(int imod = 0; imod < nMod; imod++)
		{
			gSystem->ProcessEvents();
			bcount[imod] = fadc->FADC500IBSread_BCOUNT(sid[imod]);
			printf("Mod[%d] bcount = %ld\n",sid[imod], bcount[imod]);

			if (bcount[imod])
			{
				fadc->FADC500IBSread_DATA(sid[imod], CHUNK_SIZE, data[imod]);
				fwrite(data[imod], 1, CHUNK_SIZE*1024, fp);
				gSystem->ProcessEvents();
			}
		}
		k = 0;
		while (k < CHUNK_SIZE*1024)
		{
			gSystem->ProcessEvents();
			for (int imod = 0; imod < nMod; imod++)
			{
				gSystem->ProcessEvents();
				if (bcount[imod])
				{
					gSystem->ProcessEvents();
					data_length =  data[imod][k] & 0xFF;
					itmp = data[imod][k+1] & 0xFF;
					data_length = data_length + (unsigned int)(itmp << 8);
					itmp = data[imod][k+2] & 0xFF;
					data_length = data_length + (unsigned int)(itmp << 16);
					itmp = data[imod][k+3] & 0xFF;
					data_length = data_length + (unsigned int)(itmp << 24);

					run_number = data[imod][k+4] & 0xFF;
					itmp = data[imod][k+5] & 0xFF;
					run_number = run_number + (unsigned int)(itmp << 8);

					trigger_type = data[imod][k+6] & 0x0F;

					itmp = data[imod][k+6] & 0xF0;
					trigger_destination = itmp >> 4;

					trigger_number = data[imod][k+7] & 0xFF;
					itmp = data[imod][k+8] & 0xFF;
					trigger_number = trigger_number + (unsigned int)(itmp << 8);
					itmp = data[imod][k+9] & 0xFF;
					trigger_number = trigger_number + (unsigned int)(itmp << 16);
					itmp = data[imod][k+10] & 0xFF;
					trigger_number = trigger_number + (unsigned int)(itmp << 24);					

					ttime = data[imod][k+11] & 0xFF;
					ttime = ttime*8;

					ltmp = data[imod][k+12] & 0xFF;
					ttime = ttime + ltmp * 1000;
					ltmp = data[imod][k+13] & 0xFF;
					ltmp = ltmp << 8;
					ttime = ttime + ltmp * 1000;
					ltmp = data[imod][k+14] & 0xFF;
					ltmp = ltmp << 16;
					ttime = ttime + ltmp * 1000;

					mid = data[imod][k+15] & 0xFF;

					channel = data[imod][k+16] & 0xFF;

					local_tnum = data[imod][k+17] & 0xFF;
					itmp = data[imod][k+18] & 0xFF;
					local_tnum = local_tnum + (unsigned int)(itmp << 8);
					itmp = data[imod][k+19] & 0xFF;
					local_tnum = local_tnum + (unsigned int)(itmp << 16);
					itmp = data[imod][k+20] & 0xFF;
					local_tnum = local_tnum + (unsigned int)(itmp << 24);

					trigger_pattern = data[imod][k+21] & 0xFF;
					itmp = data[imod][k+22] & 0xFF;
					trigger_pattern = trigger_pattern + (unsigned int)(itmp << 8);
					itmp = data[imod][k+23] & 0xFF;
					trigger_pattern = trigger_pattern + (unsigned int)(itmp << 16);
					itmp = data[imod][k+24] & 0xFF;
					trigger_pattern = trigger_pattern + (unsigned int)(itmp << 24);

					ltime = data[imod][k+25] & 0xFF;
					ltime = ltime*8;

					ltmp = data[imod][k+26] & 0xFF;
					ltime = ltime + ltmp*1000;
					ltmp = data[imod][k+27] & 0xFF;
					ltmp = ltmp << 8;
					ltime = ltime + ltmp*1000;
					ltmp = data[imod][k+28] & 0xFF;
					ltmp = ltmp << 16;
					ltime = ltime + ltmp*1000;

					trig_timel = data[imod][k+11] & 0xFF;

					ltmp = data[imod][k+12] & 0xFF;
					trig_timeh = ltmp;
					ltmp = data[imod][k+13] & 0xFF;
					ltmp = ltmp << 8;
					trig_timeh = trig_timeh + ltmp;
					ltmp = data[imod][k+14] & 0xFF;
					ltmp = ltmp << 16;
					trig_timeh = trig_timeh + ltmp;

					start_timel = data[imod][k+25] & 0xFF;

					ltmp = data[imod][k+26] & 0xFF;
					start_timeh = ltmp;
					ltmp = data[imod][k+27] & 0xFF;
					ltmp = ltmp << 8;
					start_timeh = start_timeh + ltmp;
					ltmp = data[imod][k+28] & 0xFF;
					ltmp = ltmp << 16;
					start_timeh = start_timeh + ltmp;
					hist_point = (data_length - 32)/2;
					hist_range = hist_point * 2;
					gSystem->ProcessEvents();
					if (adcflag == 1)
					{
						if (hoscd1 == 0)
						{
							hoscd1 = new TH1F("hoscd1", "Channel1", hist_point, 0, hist_range);
							hoscd2 = new TH1F("hoscd2", "Channel2", hist_point, 0, hist_range);
							hoscd3 = new TH1F("hoscd3", "Channel3", hist_point, 0, hist_range);
							hoscd4 = new TH1F("hoscd4", "Channel4", hist_point, 0, hist_range);
							c1->cd(1);
							hoscd1->Draw();
							c1->cd(2);
							hoscd2->Draw();
							c1->cd(3);
							hoscd3->Draw();
							c1->cd(4);
							hoscd4->Draw();
							gSystem->ProcessEvents();
						}

						else
						{
							hoscd1->Reset();
							hoscd2->Reset();
							hoscd3->Reset();
							hoscd4->Reset();

							for (int j=0; j<hist_point; j++)
							{
								adc = data[imod][k+32+j*2] & 0xFF;
								itmp = data[imod][k+32+j*2+1] & 0x0F;
								adc = adc + (unsigned int)(itmp << 8);

								if (channel == 1)
									hoscd1->Fill(j*2, adc);
								else if (channel == 2)
									hoscd2->Fill(j*2, adc);
								else if (channel == 3)
									hoscd3->Fill(j*2, adc);
								else if (channel == 4)
									hoscd4->Fill(j*2, adc);
							}
							c1->cd(1);
							hoscd1->Draw();
							hoscd1->Sumw2(kFALSE);
							c1->cd(2);
							hoscd2->Draw();
							hoscd2->Sumw2(kFALSE);
							c1->cd(3);
							hoscd3->Draw();
							hoscd3->Sumw2(kFALSE);
							c1->cd(4);
							hoscd4->Draw();
							hoscd4->Sumw2(kFALSE);
							c1->Modified();
							c1->Update();
							gSystem->ProcessEvents();
						}
					}

					if (tdcflag == 1)
					{
						if (hostd1 == 0)
						{
							hostd1 = new TH1F("hostd1", "Channel1", hist_point / 4, 0, hist_range);
							hostd2 = new TH1F("hostd2", "Channel2", hist_point / 4, 0, hist_range);
							hostd3 = new TH1F("hostd3", "Channel3", hist_point / 4, 0, hist_range);
							hostd4 = new TH1F("hostd4", "Channel4", hist_point / 4, 0, hist_range);
							c2->cd(1);
							hostd1->Draw();
							c2->cd(2);
							hostd2->Draw();
							c2->cd(3);
							hostd3->Draw();
							c2->cd(4);
							hostd4->Draw();
							gSystem->ProcessEvents();
						}
						else
						{
							hostd1->Reset();
							hostd2->Reset();
							hostd3->Reset();
							hostd4->Reset();

							for (j=0; j<(hist_point / 4); j++)
							{
								tdc = (data[imod][k+32+j*8+1] >> 4) & 0xF;
								itmp = (data[imod][k+32+j*8+3] >> 4) & 0xF;
								tdc = tdc + (unsigned int)(itmp << 4);
								itmp = (data[imod][k+32+j*8+5] >> 4) & 0x3;
								tdc = tdc + (unsigned int)(itmp << 8);

								if (channel == 1)
									hostd1->Fill(j*8, tdc);
								else if (channel == 2)
									hostd2->Fill(j*8, tdc);
								else if (channel == 3)
									hostd3->Fill(j*8, tdc);
								else if (channel == 4)
									hostd4->Fill(j*8, tdc);
							}
							tdc = hostd1->GetMinimum();
							c2->cd(1);
							hostd1->Draw();
							hostd1->Sumw2(kFALSE);
							c2->cd(2);
							hostd2->Draw();
							hostd2->Sumw2(kFALSE);
							c2->cd(3);
							hostd3->Draw();
							hostd3->Sumw2(kFALSE);
							c2->cd(4);
							hostd4->Draw();
							hostd4->Sumw2(kFALSE);
							c2->Modified();
							c2->Update();
							gSystem->ProcessEvents();
						}
					}
					printf("module ID = %d, channel ID = %d\n", mid, channel);
					printf("data_length = %d, run_number = %d, trigger_type = %d, trigger_destination = %d\n", data_length, run_number, trigger_type, trigger_destination);
					printf("trigger_number = %d, local_tnum = %d, trigger_pattern = %d\n", trigger_number, local_tnum, trigger_pattern);
					printf("trigger time = %ld, local starting time = %ld\n", ttime, ltime);
					printf("-------------------------------------------------------------------------------------------------------\n");
					fprintf(lfp, "%lX  %lX  %lX  %lX  %d\n", trig_timel, trig_timeh, start_timel, start_timeh, adc);
				}
				if (iEvent >= nEvent)	break;
				gSystem->ProcessEvents();

				if (flag == 0)	break;			
				gSystem->ProcessEvents();

				gSystem->ProcessEvents();
			}
			k = k + data_length;
			iEvent++;
			gSystem->ProcessEvents();

			if (iEvent >= nEvent)	break;
			gSystem->ProcessEvents();

			if (flag == 0)	break;			
			gSystem->ProcessEvents();
		}	
		if (iEvent >= nEvent)	break;
		gSystem->ProcessEvents();

		if (flag == 0)	break;			
		gSystem->ProcessEvents();
	}
	gSystem->ProcessEvents();

	fclose(fp);
	fclose(lfp);
	printf("Data file has been saved.\n");

	for (int imod = 0; imod < nMod; imod++)
	{
		fadc->FADC500IBSclose(sid[imod]);
	}
	usb->USB3Exit(0);

	return 0;

}
