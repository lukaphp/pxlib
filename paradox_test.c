#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paradox.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <paradox_file.db>\n", argv[0]);
        return 1;
    }

    // Initialize the PX library
    PX_INIT;

    // Create a new Paradox object
    pxdoc_t *pxdoc = PX_new();
    if (pxdoc == NULL)
    {
        fprintf(stderr, "Could not create Paradox object\n");
        return 1;
    }

    // Open the Paradox database file
    if (PX_open_file(pxdoc, argv[1]) < 0)
    {
        fprintf(stderr, "Could not open Paradox file %s\n", argv[1]);
        PX_delete(pxdoc);
        return 1;
    }

    // Get some basic information about the database
    int numFields = PX_get_num_fields(pxdoc);
    int numRecords = PX_get_num_records(pxdoc);

    printf("Database information:\n");
    printf("Number of fields: %d\n", numFields);
    printf("Number of records: %d\n", numRecords);

    // Print field information
    printf("\nField information:\n");
    for (int i = 0; i < numFields; i++)
    {
        char *fieldName;
        int fieldType;
        int fieldSize;

        PX_get_field(pxdoc, i, &fieldName, &fieldType, &fieldSize, NULL);
        printf("Field %d: Name='%s', Type=%d, Size=%d\n",
               i, fieldName, fieldType, fieldSize);
    }

    // Read and print first 5 records (or less if fewer exist)
    printf("\nFirst records:\n");
    int recordsToRead = (numRecords < 5) ? numRecords : 5;

    for (int i = 0; i < recordsToRead; i++)
    {
        void *data = PX_retrieve_record(pxdoc, i);
        if (data)
        {
            printf("\nRecord %d:\n", i + 1);
            for (int j = 0; j < numFields; j++)
            {
                char *fieldName;
                int fieldType;

                PX_get_field(pxdoc, j, &fieldName, &fieldType, NULL, NULL);

                // Handle different field types
                switch (fieldType)
                {
                case pxfAlpha:
                {
                    char *value = PX_get_data_alpha(pxdoc, data, j);
                    printf("  %s = %s\n", fieldName, value ? value : "NULL");
                    if (value)
                        free(value);
                    break;
                }
                case pxfShort:
                case pxfLong:
                {
                    long value = PX_get_data_long(pxdoc, data, j);
                    printf("  %s = %ld\n", fieldName, value);
                    break;
                }
                case pxfNumber:
                case pxfCurrency:
                {
                    double value = PX_get_data_double(pxdoc, data, j);
                    printf("  %s = %.2f\n", fieldName, value);
                    break;
                }
                case pxfDate:
                {
                    long value = PX_get_data_long(pxdoc, data, j);
                    printf("  %s = %ld (date)\n", fieldName, value);
                    break;
                }
                default:
                    printf("  %s = (unsupported type %d)\n", fieldName, fieldType);
                }
            }
            free(data);
        }
    }

    // Clean up
    PX_close(pxdoc);
    PX_delete(pxdoc);

    return 0;
}