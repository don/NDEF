#ifndef NdefMessage_h
#define NdefMessage_h

#include <Ndef.h>
#include <NdefRecord.h>

#define MAX_NDEF_RECORDS 4

class NdefMessage
{
    public:
        NdefMessage(void);
        NdefMessage(const byte *data, const int numBytes);
        NdefMessage(const NdefMessage& rhs);
        ~NdefMessage();
        NdefMessage& operator=(const NdefMessage& rhs);

        int getEncodedSize(); // need so we can pass array to encode
        void encode(byte *data);

        boolean addRecord(NdefRecord& record);
        void addMimeMediaRecord(String mimeType, String payload);
        void addMimeMediaRecord(String mimeType, byte *payload, int payloadLength);
        void addTextRecord(String text);
        void addTextRecord(String text, String encoding);
        void addUriRecord(String uri);

		/** 
		 * Creates an Android Application Record (AAR) http://developer.android.com/guide/topics/connectivity/nfc/nfc.html#aar
		 * Use an AAR record to cause a P2P message pushed to your Android phone to launch your app even if it's not running.
		 * Note, Android version must be >= 4.0 and your app must have the package you pass to this method
		 * 
		 * @param packageName example: "com.acme.myapp" 
		 */
        void addAndroidApplicationRecord(char *packageName);

        void addUnknownRecord(byte *payload, int payloadLength);
        void addEmptyRecord();

        unsigned int getRecordCount();
        NdefRecord getRecord(int index);
        NdefRecord operator[](int index);

#ifdef NDEF_USE_SERIAL
        void print();
#endif
    private:
        NdefRecord _records[MAX_NDEF_RECORDS];
        unsigned int _recordCount;
};

#endif
