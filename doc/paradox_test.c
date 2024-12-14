#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <include/paradox.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <paradox_file.db>\n", argv[0]);
        return 1;
    }

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
        pxfield_t *field = PX_get_field(pxdoc, i);
        if (field)
        {
            printf("Field %d: Name='%s', Type=%d, Size=%d\n",
                   i, field->px_fname, field->px_ftype, field->px_flen);
        }
    }

    // Read and print first 5 records (or less if fewer exist)
    printf("\nFirst records:\n");
    int recordsToRead = (numRecords < 5) ? numRecords : 5;

    for (int i = 0; i < recordsToRead; i++)
    {
        pxval_t **data = PX_retrieve_record(pxdoc, i);
        if (data)
        {
            printf("\nRecord %d:\n", i + 1);
            for (int j = 0; j < numFields; j++)
            {
                pxfield_t *field = PX_get_field(pxdoc, j);
                if (!field)
                    continue;

                // Handle different field types
                switch (field->px_ftype)
                {
                case pxfAlpha:
                {
                    char *value = NULL;
                    if (PX_get_data_alpha(pxdoc, (char *)data[j], field->px_flen, &value) == 0)
                    {
                        printf("  %s = %s\n", field->px_fname, value ? value : "NULL");
                        if (value)
                            free(value);
                    }
                    break;
                }
                case pxfShort:
                case pxfLong:
                {
                    long value;
                    if (PX_get_data_long(pxdoc, (char *)data[j], field->px_flen, &value) == 0)
                    {
                        printf("  %s = %ld\n", field->px_fname, value);
                    }
                    break;
                }
                case pxfNumber:
                case pxfCurrency:
                {
                    double value;
                    if (PX_get_data_double(pxdoc, (char *)data[j], field->px_flen, &value) == 0)
                    {
                        printf("  %s = %.2f\n", field->px_fname, value);
                    }
                    break;
                }
                case pxfDate:
                {
                    long value;
                    if (PX_get_data_long(pxdoc, (char *)data[j], field->px_flen, &value) == 0)
                    {
                        printf("  %s = %ld (date)\n", field->px_fname, value);
                    }
                    break;
                }
                default:
                    printf("  %s = (unsupported type %d)\n", field->px_fname, field->px_ftype);
                }
            }
            // Free the record data
            for (int j = 0; j < numFields; j++)
            {
                if (data[j])
                {
                    free(data[j]);
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
