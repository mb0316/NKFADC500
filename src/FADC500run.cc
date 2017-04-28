#include "TROOT.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "TCanvas.h"
#include "TH1F.h"
#include "FADC500run.h"
#include "FADC500setting.h"
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
	FADC500setting fset;
	const int nMod = nModule;
	vector <int> sid;
	for (i = 0; i < nMod; i++)
	{
		sid.push_back(i+1);
		std::cout << "Serial ID : " << sid[i] << std::endl;
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

	fp = fopen(filename, "wb");
	lfp = fopen(lfilename, "wt");


	flag = 1;
	evtn = 0;

	while (flag)
	{
		if (flag == 0) break;
		
		gSystem->ProcessEvents();
		for(int imod = 0; imod< nMod; imod++)
		{
			bcount = fadc->FADC500IBSread_BCOUNT(sid[imod]);
			printf("Mod[%d] bcount = %ld\n",sid[imod], bcount);

			gSystem->ProcessEvents();
			if (bcount)
			{
				chunk = bcount / CHUNK_SIZE;
				slice = bcount % CHUNK_SIZE;
				slice = slice / 32;
				slice = slice * 32;
				gSystem->ProcessEvents();

				for (i = 0; i < chunk; i++)
				{
					fadc->FADC500IBSread_DATA(sid[imod], CHUNK_SIZE, data);

					fwrite(data, 1, CHUNK_SIZE * 1024, fp);
					gSystem->ProcessEvents();

					k = 0;
					while (k < CHUNK_SIZE*1024)
					{
						gSystem->ProcessEvents();
						data_length =  data[k] & 0xFF;
						itmp = data[k+1] & 0xFF;
						data_length = data_length + (unsigned int)(itmp << 8);
						itmp = data[k+2] & 0xFF;
						data_length = data_length + (unsigned int)(itmp << 16);
						itmp = data[k+3] & 0xFF;
						data_length = data_length + (unsigned int)(itmp << 24);

						run_number = data[k+4] & 0xFF;
						itmp = data[k+5] & 0xFF;
						run_number = run_number + (unsigned int)(itmp << 8);

						trigger_type = data[k+6] & 0x0F;

						itmp = data[k+6] & 0xF0;
						trigger_destination = itmp >> 4;

						trigger_number = data[k+7] & 0xFF;
						itmp = data[k+8] & 0xFF;
						trigger_number = trigger_number + (unsigned int)(itmp << 8);
						itmp = data[k+9] & 0xFF;
						trigger_number = trigger_number + (unsigned int)(itmp << 16);
						itmp = data[k+10] & 0xFF;
						trigger_number = trigger_number + (unsigned int)(itmp << 24);					

						ttime = data[k+11] & 0xFF;
						ttime = ttime*8;

						ltmp = data[k+12] & 0xFF;
						ttime = ttime + ltmp * 1000;
						ltmp = data[k+13] & 0xFF;
						ltmp = ltmp << 8;
						ttime = ttime + ltmp * 1000;
						ltmp = data[k+14] & 0xFF;
						ltmp = ltmp << 16;
						ttime = ttime + ltmp * 1000;

						mid = data[k+15] & 0xFF;

						channel = data[k+16] & 0xFF;

						local_tnum = data[k+17] & 0xFF;
						itmp = data[k+18] & 0xFF;
						local_tnum = local_tnum + (unsigned int)(itmp << 8);
						itmp = data[k+19] & 0xFF;
						local_tnum = local_tnum + (unsigned int)(itmp << 16);
						itmp = data[k+20] & 0xFF;
						local_tnum = local_tnum + (unsigned int)(itmp << 24);

						trigger_pattern = data[k+21] & 0xFF;
						itmp = data[k+22] & 0xFF;
						trigger_pattern = trigger_pattern + (unsigned int)(itmp << 8);
						itmp = data[k+23] & 0xFF;
						trigger_pattern = trigger_pattern + (unsigned int)(itmp << 16);
						itmp = data[k+24] & 0xFF;
						trigger_pattern = trigger_pattern + (unsigned int)(itmp << 24);

						ltime = data[k+25] & 0xFF;
						ltime = ltime*8;

						ltmp = data[k+26] & 0xFF;
						ltime = ltime + ltmp*1000;
						ltmp = data[k+27] & 0xFF;
						ltmp = ltmp << 8;
						ltime = ltime + ltmp*1000;
						ltmp = data[k+28] & 0xFF;
						ltmp = ltmp << 16;
						ltime = ltime + ltmp*1000;

						trig_timel = data[k+11] & 0xFF;

						ltmp = data[k+12] & 0xFF;
						trig_timeh = ltmp;
						ltmp = data[k+13] & 0xFF;
						ltmp = ltmp << 8;
						trig_timeh = trig_timeh + ltmp;
						ltmp = data[k+14] & 0xFF;
						ltmp = ltmp << 16;
						trig_timeh = trig_timeh + ltmp;

						start_timel = data[k+25] & 0xFF;

						ltmp = data[k+26] & 0xFF;
						start_timeh = ltmp;
						ltmp = data[k+27] & 0xFF;
						ltmp = ltmp << 8;
						start_timeh = start_timeh + ltmp;
						ltmp = data[k+28] & 0xFF;
						ltmp = ltmp << 16;
						start_timeh = start_timeh + ltmp;
						std::cout<< "data_length" << std::endl;
						std::cout<< data_length << std::endl;
						std::cout<< "//////////" << std::endl;
						hist_point = (data_length - 32)/2;
						hist_range = hist_point * 2;
						gSystem->ProcessEvents();
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
								adc = data[k+32+j*2] & 0xFF;
								itmp = data[k+32+j*2+1] & 0x0F;
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
								tdc = (data[k+32+j*8+1] >> 4) & 0xF;
								itmp = (data[k+32+j*8+3] >> 4) & 0xF;
								tdc = tdc + (unsigned int)(itmp << 4);
								itmp = (data[k+32+j*8+5] >> 4) & 0x3;
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

						printf("data_length = %d, run_number = %d, trigger_type = %d, trigger_destination = %d\n", data_length, run_number, trigger_type, trigger_destination);
						printf("trigger_number = %d, local_tnum = %d, trigger_pattern = %d\n", trigger_number, local_tnum, trigger_pattern);
						printf("trigger time = %ld, local starting time = %ld\n", ttime, ltime);
						printf("-------------------------------------------------------------------------------------------------------\n");
						fprintf(lfp, "%lX  %lX  %lX  %lX  %d\n",
								trig_timel, trig_timeh, start_timel, start_timeh, adc);
						k = k + data_length;
						evtn++;
						gSystem->ProcessEvents();
						if (flag == 0) break;
						if ( evtn >= nEvent)
						{
							i = chunk;
							slice = 0;
							flag = 0;
							break;
						}
						gSystem->ProcessEvents();
					}
					gSystem->ProcessEvents();

				}


				if (slice)	
				{
					gSystem->ProcessEvents();
					fadc->FADC500IBSread_DATA(sid[imod], slice, data);

					fwrite(data, 1, slice * 1024, fp);

					k = 0;
					while (k < slice*1024)
					{
						gSystem->ProcessEvents();
						data_length =  data[k] & 0xFF;
						itmp = data[k+1] & 0xFF;
						data_length = data_length + (unsigned int)(itmp << 8);
						itmp = data[k+2] & 0xFF;
						data_length = data_length + (unsigned int)(itmp << 16);
						itmp = data[k+3] & 0xFF;
						data_length = data_length + (unsigned int)(itmp << 24);

						run_number = data[k+4] & 0xFF;
						itmp = data[k+5] & 0xFF;
						run_number = run_number + (unsigned int)(itmp << 8);

						trigger_type = data[k+6] & 0x0F;

						itmp = data[k+6] & 0xF0;
						trigger_destination = itmp >> 4;

						trigger_number = data[k+7] & 0xFF;
						itmp = data[k+8] & 0xFF;
						trigger_number = trigger_number + (unsigned int)(itmp << 8);
						itmp = data[k+9] & 0xFF;
						trigger_number = trigger_number + (unsigned int)(itmp << 16);
						itmp = data[k+10] & 0xFF;
						trigger_number = trigger_number + (unsigned int)(itmp << 24);					

						ttime = data[k+11] & 0xFF;
						ttime = ttime*8;

						ltmp = data[k+12] & 0xFF;
						ttime = ttime + ltmp * 1000;
						ltmp = data[k+13] & 0xFF;
						ltmp = ltmp << 8;
						ttime = ttime + ltmp * 1000;
						ltmp = data[k+14] & 0xFF;
						ltmp = ltmp << 16;
						ttime = ttime + ltmp * 1000;

						mid = data[k+15] & 0xFF;

						channel = data[k+16] & 0xFF;

						local_tnum = data[k+17] & 0xFF;
						itmp = data[k+18] & 0xFF;
						local_tnum = local_tnum + (unsigned int)(itmp << 8);
						itmp = data[k+19] & 0xFF;
						local_tnum = local_tnum + (unsigned int)(itmp << 16);
						itmp = data[k+20] & 0xFF;
						local_tnum = local_tnum + (unsigned int)(itmp << 24);

						trigger_pattern = data[k+21] & 0xFF;
						itmp = data[k+22] & 0xFF;
						trigger_pattern = trigger_pattern + (unsigned int)(itmp << 8);
						itmp = data[k+23] & 0xFF;
						trigger_pattern = trigger_pattern + (unsigned int)(itmp << 16);
						itmp = data[k+24] & 0xFF;
						trigger_pattern = trigger_pattern + (unsigned int)(itmp << 24);

						ltime = data[k+25] & 0xFF;
						ltime = ltime*8;

						ltmp = data[k+26] & 0xFF;
						ltime = ltime + ltmp*1000;
						ltmp = data[k+27] & 0xFF;
						ltmp = ltmp << 8;
						ltime = ltime + ltmp*1000;
						ltmp = data[k+28] & 0xFF;
						ltmp = ltmp << 16;
						ltime = ltime + ltmp*1000;

						trig_timel = data[k+11] & 0xFF;

						ltmp = data[k+12] & 0xFF;
						trig_timeh = ltmp;
						ltmp = data[k+13] & 0xFF;
						ltmp = ltmp << 8;
						trig_timeh = trig_timeh + ltmp;
						ltmp = data[k+14] & 0xFF;
						ltmp = ltmp << 16;
						trig_timeh = trig_timeh + ltmp;

						start_timel = data[k+25] & 0xFF;

						ltmp = data[k+26] & 0xFF;
						start_timeh = ltmp;
						ltmp = data[k+27] & 0xFF;
						ltmp = ltmp << 8;
						start_timeh = start_timeh + ltmp;
						ltmp = data[k+28] & 0xFF;
						ltmp = ltmp << 16;
						start_timeh = start_timeh + ltmp;
						std::cout<< "data_length" << std::endl;
						std::cout<< data_length << std::endl;
						std::cout<< "//////////" << std::endl;
						hist_point = (data_length - 32)/2;
						hist_range = hist_point * 2;
						gSystem->ProcessEvents();
						if (hoscd1 == 0)
						{
							hoscd1 = new TH1F("hoscd1", "Channel1", hist_point, 0, hist_range);
							hoscd2 = new TH1F("hoscd2", "Channel2", hist_point, 0, hist_range);
							hoscd3 = new TH1F("hoscd3", "Channel3", hist_point, 0, hist_range);
							hoscd4 = new TH1F("hoscd4", "Channel4", hist_point, 0, hist_range);
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
								adc = data[k+32+j*2] & 0xFF;
								itmp = data[k+32+j*2+1] & 0x0F;
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
								tdc = (data[k+32+j*8+1] >> 4) & 0xF;
								itmp = (data[k+32+j*8+3] >> 4) & 0xF;
								tdc = tdc + (unsigned int)(itmp << 4);
								itmp = (data[k+32+j*8+5] >> 4) & 0x3;
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

						printf("data_length = %d, run_number = %d, trigger_type = %d, trigger_destination = %d\n", data_length, run_number, trigger_type, trigger_destination);
						printf("trigger_number = %d, local_tnum = %d, trigger_pattern = %d\n", trigger_number, local_tnum, trigger_pattern);
						printf("trigger time = %ld, local starting time = %ld\n", ttime, ltime);
						printf("-------------------------------------------------------------------------------------------------------\n");
						fprintf(lfp, "%lX  %lX  %lX  %lX  %d\n",
								trig_timel, trig_timeh, start_timel, start_timeh, adc);
						k = k + data_length;
						evtn++;
						gSystem->ProcessEvents();
						if (flag == 0) break;
						if ( evtn >= nEvent)
						{
							flag = 0;
							break;
						}
						gSystem->ProcessEvents();
					}
					gSystem->ProcessEvents();

				}
				gSystem->ProcessEvents();

			}	
			gSystem->ProcessEvents();
		}
		gSystem->ProcessEvents();
	}
	gSystem->ProcessEvents();

	fclose(fp);
	fclose(lfp);

	for (int imod = 0; imod < nMod; imod++)
	{
		fadc->FADC500IBSclose(sid[imod]);
	}
	usb->USB3Exit(0);

	return 0;

}


