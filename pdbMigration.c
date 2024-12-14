#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <paradox.h>

// Function to convert from modified big endian to native format
void convert_from_paradox_format(char *data, int len)
{
    // Complement the sign bit (highest bit of first byte)
    data[0] ^= 0x80;

    // Reverse the bytes for little endian systems
    for (int i = 0; i < len / 2; i++)
    {
        char temp = data[i];
        data[i] = data[len - 1 - i];
        data[len - 1 - i] = temp;
    }
}

void print_field_info(pxdoc_t *pxdoc, int num_fields)
{
    printf("\nField Information:\n");
    printf("%-20s %-10s %-10s\n", "Name", "Type", "Size");
    printf("----------------------------------------\n");

    for (int i = 0; i < num_fields; i++)
    {
        pxfield_t *field = PX_get_field(pxdoc, i);
        if (field)
        {
            const char *type_name;
            switch (field->px_ftype)
            {
            case pxfAlpha:
                type_name = "Alpha";
                break;
            case pxfDate:
                type_name = "Date";
                break;
            case pxfShort:
                type_name = "Short";
                break;
            case pxfLong:
                type_name = "Long";
                break;
            case pxfCurrency:
                type_name = "Currency";
                break;
            case pxfNumber:
                type_name = "Number";
                break;
            case pxfLogical:
                type_name = "Logical";
                break;
            case pxfMemoBLOb:
                type_name = "Memo";
                break;
            case pxfBLOb:
                type_name = "Blob";
                break;
            case pxfFmtMemoBLOb:
                type_name = "FmtMemo";
                break;
            case pxfOLE:
                type_name = "OLE";
                break;
            case pxfGraphic:
                type_name = "Graphic";
                break;
            case pxfTime:
                type_name = "Time";
                break;
            case pxfTimestamp:
                type_name = "Timestamp";
                break;
            case pxfAutoInc:
                type_name = "AutoInc";
                break;
            case pxfBCD:
                type_name = "BCD";
                break;
            case pxfBytes:
                type_name = "Bytes";
                break;
            default:
                type_name = "Unknown";
            }
            printf("%-20s %-10s %-10d\n", field->px_fname, type_name, field->px_flen);
        }
    }
    printf("\n");
}

void write_field_value(FILE *out, pxdoc_t *pxdoc, pxval_t *data, pxfield_t *field, int debug)
{
    if (!field)
    {
        fprintf(out, "\"\"");
        return;
    }

    // Skip blob data types immediately
    if (field->px_ftype == pxfMemoBLOb ||
        field->px_ftype == pxfBLOb ||
        field->px_ftype == pxfFmtMemoBLOb ||
        field->px_ftype == pxfOLE ||
        field->px_ftype == pxfGraphic)
    {
        if (debug)
        {
            printf("Field %s (Blob): Skipped\n", field->px_fname);
        }
        fprintf(out, "\"\"");
        return;
    }

    // Handle NULL data
    if (!data || data->isnull)
    {
        fprintf(out, "\"\"");
        return;
    }

    switch (field->px_ftype)
    {
    case pxfAlpha:
        if (data->value.str.val)
        {
            if (debug)
            {
                printf("Field %s (Alpha): value=%s\n",
                       field->px_fname, data->value.str.val);
            }

            // Trim trailing nulls and whitespace
            char *str = data->value.str.val;
            int len = data->value.str.len;
            while (len > 0 && (str[len - 1] == '\0' || isspace(str[len - 1])))
                len--;

            // Escape quotes and output
            fprintf(out, "\"");
            for (int i = 0; i < len; i++)
            {
                if (str[i] == '"')
                    fprintf(out, "\"\"");
                else
                    fputc(str[i], out);
            }
            fprintf(out, "\"");
        }
        else
        {
            fprintf(out, "\"\"");
        }
        break;
    case pxfShort:
    case pxfLong:
    case pxfAutoInc:
        if (debug)
        {
            printf("Field %s (Long): value=%ld\n", field->px_fname, data->value.lval);
        }
        fprintf(out, "%ld", data->value.lval);
        break;
    case pxfCurrency:
    case pxfNumber:
        if (debug)
        {
            printf("Field %s (Number): value=%f\n", field->px_fname, data->value.dval);
        }
        if (field->px_ftype == pxfCurrency)
            fprintf(out, "%.2f", data->value.dval); // Currency always shows 2 decimal places
        else
            fprintf(out, "%.6f", data->value.dval); // Regular numbers show up to 6 decimal places
        break;
    case pxfLogical:
        if (debug)
        {
            printf("Field %s (Logical): value=%ld\n", field->px_fname, data->value.lval);
        }
        fprintf(out, "\"%s\"", data->value.lval ? "true" : "false");
        break;
    case pxfDate:
    {
        if (debug)
        {
            printf("Field %s (Date): value=%ld\n", field->px_fname, data->value.lval);
        }
        char *date_str = PX_date2string(pxdoc, data->value.lval, "YYYY-MM-DD");
        if (date_str)
        {
            fprintf(out, "\"%s\"", date_str);
            free(date_str);
        }
        else
        {
            fprintf(out, "\"\"");
        }
        break;
    }
    case pxfTime:
    {
        if (debug)
        {
            printf("Field %s (Time): value=%ld\n", field->px_fname, data->value.lval);
        }
        char *time_str = PX_time2string(pxdoc, data->value.lval, "HH:MI:SS");
        if (time_str)
        {
            fprintf(out, "\"%s\"", time_str);
            free(time_str);
        }
        else
        {
            fprintf(out, "\"\"");
        }
        break;
    }
    case pxfTimestamp:
    {
        if (debug)
        {
            printf("Field %s (Timestamp): value=%f\n", field->px_fname, data->value.dval);
        }
        char *timestamp_str = PX_timestamp2string(pxdoc, data->value.dval, "YYYY-MM-DD HH:MI:SS");
        if (timestamp_str)
        {
            fprintf(out, "\"%s\"", timestamp_str);
            free(timestamp_str);
        }
        else
        {
            fprintf(out, "\"\"");
        }
        break;
    }
    case pxfBCD:
        if (data->value.str.val)
        {
            if (debug)
            {
                printf("Field %s (BCD): value=%s\n", field->px_fname, data->value.str.val);
            }
            fprintf(out, "\"%s\"", data->value.str.val);
        }
        else
        {
            fprintf(out, "\"\"");
        }
        break;
    default:
        fprintf(out, "\"\"");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <paradox_file.db>\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    char output_file[1024];
    snprintf(output_file, sizeof(output_file), "%s.csv", input_file);

    pxdoc_t *pxdoc = PX_new();
    if (!pxdoc)
    {
        fprintf(stderr, "Could not create Paradox object\n");
        return 1;
    }

    if (PX_open_file(pxdoc, input_file) < 0)
    {
        fprintf(stderr, "Could not open Paradox file %s\n", input_file);
        PX_delete(pxdoc);
        return 1;
    }

    FILE *out = fopen(output_file, "w");
    if (!out)
    {
        fprintf(stderr, "Could not create output file %s\n", output_file);
        PX_close(pxdoc);
        PX_delete(pxdoc);
        return 1;
    }

    int num_fields = PX_get_num_fields(pxdoc);
    int num_records = PX_get_num_records(pxdoc);

    printf("Processing database:\n");
    printf("Fields: %d\n", num_fields);
    printf("Records: %d\n", num_records);

    // Print detailed field information
    print_field_info(pxdoc, num_fields);

    // Write CSV header
    for (int i = 0; i < num_fields; i++)
    {
        pxfield_t *field = PX_get_field(pxdoc, i);
        if (field && field->px_fname)
        {
            fprintf(out, "\"%s\"%s", field->px_fname, (i < num_fields - 1) ? "," : "\n");
        }
        else
        {
            fprintf(out, "\"Field_%d\"%s", i + 1, (i < num_fields - 1) ? "," : "\n");
        }
    }

    // Process records using PX_retrieve_record
    for (int rec = 0; rec < num_records; rec++)
    {
        pxval_t **record = PX_retrieve_record(pxdoc, rec);
        if (record)
        {
            // Enable debug output for first record only
            int debug = (rec == 0);

            if (debug)
            {
                printf("\nProcessing first record (Record #%d):\n", rec);
            }

            for (int field = 0; field < num_fields; field++)
            {
                pxfield_t *field_info = PX_get_field(pxdoc, field);
                write_field_value(out, pxdoc, record[field], field_info, debug);
                if (field < num_fields - 1)
                {
                    fprintf(out, ",");
                }
            }
            fprintf(out, "\n");

            // Free the record data
            for (int j = 0; j < num_fields; j++)
            {
                if (record[j])
                {
                    free(record[j]);
                }
            }
            free(record);
        }

        if ((rec + 1) % 1000 == 0 || rec == 0)
        {
            printf("Processed %d records...\n", rec + 1);
        }
    }

    fclose(out);
    PX_close(pxdoc);
    PX_delete(pxdoc);

    printf("Conversion completed. Output written to: %s\n", output_file);
    return 0;
}
